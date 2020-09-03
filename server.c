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
#include "parser.h"
#include "database.h"
#include "messages.h"
#include "queue.h"
#include "constants.h"



pthread_mutex_t queue_mutex;  //For the server reader.
pthread_cond_t queue_non_empty;
pthread_cond_t queue_non_full; 

void signalHandler() {
    printf("Process exited by signal handler\n");
    exit(0);
}

void start_server_reader(int sock){
    char* line = NULL;
    size_t len = 0;
    ssize_t lines;
    char buffer[PROTOCOL_STANDARD_MESSAGE_LENGTH];

    //We initialize our mutex and the conditional variables
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_non_full, NULL);
    pthread_cond_init(&queue_non_empty, NULL);

    while (1) {
        if (lines = read(sock, buffer, PROTOCOL_STANDARD_MESSAGE_LENGTH) > 0) {
            printf("Received: %s\n",buffer);
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
            if(result->result_code == 300){
                break;
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

    //Creating a thread pool, which will be in charge of processing client's requests when they are received.
    pthread_t th[THREAD_POOL_NUM_THREADS];
    for(int ii=0; ii<THREAD_POOL_NUM_THREADS; ii++){
        pthread_create(&th[ii], NULL, (void *)message_manager_start, NULL);
    }
    
    start_server_reader(sock);

    //Once here, we join the writer threads, which must have ended once the 300 code was received.
    for(int ii=0; ii<THREAD_POOL_NUM_THREADS; ii++){
        pthread_join(&th[ii], NULL);
    }
}

int main(int argc, char* argv[]) {
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
    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Now bind the host address using bind() call.
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    //Registering SIGINT signal handler. The signal is received by BOTH THE PARENT AND CHILD.
    signal(SIGINT, signalHandler);

    printf("Started listening to connections\n");
    /* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
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

    } // end of while

}