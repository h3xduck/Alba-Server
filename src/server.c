#include <json-c/json.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../include/constants.h"
#include "../include/database.h"
#include "../include/messages.h"
#include "../include/parser.h"
#include "../include/queue.h"

pthread_mutex_t queue_mutex;  //For the server reader.
pthread_cond_t queue_non_empty;
pthread_cond_t queue_non_full;
pthread_mutex_t socket_stream_write_mutex;  //For the writers

void signalHandler() {
    printf("Process exited by signal handler\n");
    exit(0);
}

void start_PINGing_thread(int sock, int FLAG_CLIENT_TIMEOUT) {
}

void start_server_reader(int sock) {
    char *line = NULL;
    size_t len = 0;
    ssize_t lines;
    char buffer[PROTOCOL_STANDARD_MESSAGE_LENGTH];

    long max_seconds_without_client_notice = 5;
    int max_number_of_PINGs_with_no_answer = 3;
    long seconds_wait_between_pings = 2;
    struct timespec keepAliveTimer, timeLastConn;
    struct timespec pingTimer, timeLastPing;
    int current_cosecutive_PINGs = 0;
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeLastConn);
    clock_gettime(CLOCK_MONOTONIC_RAW, &timeLastPing);

    while (1) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &keepAliveTimer);

        //We cannot afford to be blocked on read() until the client sends a message (we need to send pings and so on)
        //so let's use select(). Note that sockSet and selTimeout are modified by select so we need to set them again each time.
        fd_set sockSet;
        struct timeval selTimeout;
        int TIMEOUT_SEC = 0;
        FD_ZERO(&sockSet);

        FD_SET(sock, &sockSet);
        selTimeout.tv_sec = TIMEOUT_SEC;
        selTimeout.tv_usec = 0;
        int res = select(sock + 1, &sockSet, NULL, NULL, &selTimeout);
        if ((res > 0) && (lines = read(sock, buffer, PROTOCOL_STANDARD_MESSAGE_LENGTH) > 0)) {
            current_cosecutive_PINGs = 0;
            clock_gettime(CLOCK_MONOTONIC_RAW, &timeLastConn);
            //printf("Received: %s\n", buffer);
            struct parser_result *result = protocol_parse(buffer);
            struct message_manager_element *element = malloc(sizeof(struct message_manager_element));
            element->sock = sock;
            element->struct_element = result;
            printf("Parser code for message is: %i\n", result->result_code);
            //printf("Parser buffer is %s\n", result->result_buffer);

            //Here we decide what to do with the already parsed message.
            //Critical section
            pthread_mutex_lock(&queue_mutex);
            queue_enqueue(element);
            pthread_mutex_unlock(&queue_mutex);
            //End of critical section

            //Once enqueued, if the code was 300, the reader must stop too (the writer threads will exit and be collected right afterwards)
            if (result->result_code == 300) {
                printf("Received code 300\n");
                break;
            }
        } else {
            long time_difference = (long)keepAliveTimer.tv_sec - timeLastConn.tv_sec;
            if (time_difference > max_seconds_without_client_notice) {
                printf("Large time difference detected: %d\n", time_difference);
                if (current_cosecutive_PINGs < max_number_of_PINGs_with_no_answer) {
                    clock_gettime(CLOCK_MONOTONIC_RAW, &pingTimer);
                    if ((long)(pingTimer.tv_sec - timeLastPing.tv_sec) > seconds_wait_between_pings) {
                        clock_gettime(CLOCK_MONOTONIC_RAW, &timeLastPing);
                        current_cosecutive_PINGs++;
                        printf("Sending PING\n");
                        sendPING(sock);
                    }else{
                        printf("Not sending PING yet...\n");
                    }
                } else if (current_cosecutive_PINGs == max_number_of_PINGs_with_no_answer) {
                    printf("%i pings without response. Closing connection\n", max_number_of_PINGs_with_no_answer);
                    break;
                }
            }
        }
        sleep(1);
    }
    printf("Finished reader thread\n");
}

/**
 * Here we start the reader thread, and wait for instructions / requests.
 */
void doprocessing(int sock) {
    printf("Starting processing\n");

    //We initialize our mutex and the conditional variables
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_non_full, NULL);
    pthread_cond_init(&queue_non_empty, NULL);

    //Creating a thread pool, which will be in charge of processing client's requests when they are received.
    pthread_t th[THREAD_POOL_NUM_THREADS];
    int *FLAG_TERMINATE_THREAD = malloc(sizeof(int));
    *FLAG_TERMINATE_THREAD = 0;
    for (int ii = 0; ii < THREAD_POOL_NUM_THREADS; ii++) {
        struct message_manager_param *thread_param = malloc(sizeof(struct message_manager_param));
        thread_param->FLAG_TERMINATE_THREAD = FLAG_TERMINATE_THREAD;
        thread_param->thread_id = ii;
        pthread_create(&th[ii], NULL, (void *)message_manager_start, thread_param);
    }

    start_server_reader(sock);

    //Once here, we join the writer threads, which must be ended once the 300 code was received.

    printf("Requesting to cancel writer threads...\n");
    *FLAG_TERMINATE_THREAD = 1;

    //The reason for using a flag and not a POSIX function such as thread_cancel is the great amount of complexity on the
    //writer thread function. Mutexes need to be released, previously enqueued requests need to be finished (not just ignored if the
    //client disconnects) and so on, so we must make sure the threads gracefully terminate instead of just terminating them.
    //Note that the great amount of cancellation points in the functions makes impredicible where the thread will end even when having set
    //PTHREAD_CANCEL_DEFERRED.

    //queue_print_positions();
    printf("Enqueueing fake elements...\n");
    for (int ii = 0; ii < THREAD_POOL_NUM_THREADS; ii++) {
        char fake_buffer[] = {"PLACEHOLDER"};
        struct parser_result *result = protocol_parse(fake_buffer);
        struct message_manager_element *element = malloc(sizeof(struct message_manager_element));
        element->sock = sock;
        element->struct_element = result;
        printf("Trying to enqueue the fake element with code %i...\n", element->struct_element->result_code);
        pthread_mutex_lock(&queue_mutex);
        queue_enqueue(element);
        pthread_mutex_unlock(&queue_mutex);
    }

    printf("Waiting for writer threads to cancel...\n");
    for (int ii = 0; ii < THREAD_POOL_NUM_THREADS; ii++) {
        pthread_join(th[ii], NULL);
        printf("Joined thread %i\n", ii);
    }

    printf("Joined all writer threads!\n");

    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_non_full);
    pthread_cond_destroy(&queue_non_empty);

    printf("Successful exit\n");
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;
    int conn_number = 0;

    // First call to socket() function
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Now bind the host address using bind() call.
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    //Registering SIGINT signal handler. The signal is received by BOTH THE PARENT AND CHILDREN.
    signal(SIGINT, signalHandler);

    printf("Started listening to connections\n");
    /* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        printf("Accepting client connection...\n");
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        printf("Success: Connection %i opened\n", conn_number++);

        // Create child process
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            // This is the client process
            close(sockfd);
            doprocessing(newsockfd);
            printf("Closing connection...\n");
            exit(0);
        } else {
            close(newsockfd);
        }

    }  // end of while
}