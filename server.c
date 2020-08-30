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

#define PROTOCOL_STANDARD_MESSAGE_LENGTH 1024

int fillerArrayLength = PROTOCOL_STANDARD_MESSAGE_LENGTH * sizeof(char);
char* fillerArray; //memory allocated at main.

void signalHandler() {
    printf("Process exited by signal handler\n");
    exit(0);
}

/**
 * 
 * 
 */
json_object* create_custom_json(struct resultStringArray resultArray) {
    json_object* jobj = json_object_new_object();
    json_object* jstring0 = json_object_new_string(resultArray.contentArray[0]);
    json_object* jstring1 = json_object_new_string(resultArray.contentArray[1]);
    json_object* jstring2 = json_object_new_string(resultArray.contentArray[2]);

    json_object_object_add(jobj, "Date", jstring0);
    json_object_object_add(jobj, "Content", jstring1);
    json_object_object_add(jobj, "User", jstring2);
    printf("The json object created: %s\n", json_object_to_json_string(jobj));

    return jobj;
}

/**
 * Generic function to send a message to the client.
 * @param sock: socket descriptor
 * @param header: header for the message
 * @param content: content to send
 */ 
void send_message(int sock, const char* header, const char* content){
    char* separator = "\n##ALBA##\n";
    int totalLength = strlen(header) + strlen(separator) + fillerArrayLength + 1;
    char* to_send = calloc(strlen(content) + strlen(header) + strlen(separator) + fillerArrayLength + 1, sizeof(char));
    printf("Sending %i bytes\n", totalLength);
    strcpy(to_send, header);
    strcat(to_send, content);
    strcat(to_send, separator);
    strcat(to_send, fillerArray);
    int n = write(sock, to_send, PROTOCOL_STANDARD_MESSAGE_LENGTH);
    printf("Sending %s\n", to_send);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    free(to_send);
}


/**
 * Sends a PONG message to the client connected to sock. 
 */ 
void sendPONG(int sock){
    send_message(sock, "PONG::", "Hello client");
}

void* reading_thread_routine(void* params) {
    int sock = *((int*)params);
    printf("Reader thread received %i as parameter\n");

    char* line = NULL;
    size_t len = 0;
    ssize_t lines;
    char buffer[PROTOCOL_STANDARD_MESSAGE_LENGTH];
    while (1) {
        if (lines = read(sock, buffer, PROTOCOL_STANDARD_MESSAGE_LENGTH) > 0) {
            struct parser_result result = protocol_parse(buffer);
            printf("Parser code for message is: %i\n", result.result_code);
            printf("Parser buffer is %s\n", result.result_buffer);
            //Here we decide what to do with the already parsed message.
            switch (result.result_code)
            {
            case 3:
                printf("Received PING\n");
                sendPONG(sock);
                break;
            default:
                break;
            }
        }
        sleep(5);
    }

    printf("Finished reader thread\n");
}

void send_init_connection_message(int sock) {
    printf("Called init connection message\n");

    char* header = "STARTCONN::";
    char* separator = "\n##ALBA##\n";
    int totalLength = strlen(header) + strlen(separator) + fillerArrayLength + 1;
    char* to_send = calloc(strlen(header) + strlen(separator) + fillerArrayLength + 1, sizeof(char));
    strcpy(to_send, header);
    strcat(to_send, separator);
    strcat(to_send, fillerArray);
    printf("Total length is %i\n", totalLength);

    int n = write(sock, to_send, PROTOCOL_STANDARD_MESSAGE_LENGTH);
    printf("Sending STARTCONN message\n");
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    free(to_send);
}

void send_end_connection_message(int sock) {
    printf("Called end connection message\n");

    char* header = "ENDCONN::";
    char* separator = "\n##ALBA##\n";
    int totalLength = strlen(header) + strlen(separator) + fillerArrayLength + 1;
    char* to_send = calloc(strlen(header) + strlen(separator) + fillerArrayLength + 1, sizeof(char));
    strcpy(to_send, header);
    strcat(to_send, separator);
    strcat(to_send, fillerArray);
    printf("Total length is %i\n", totalLength);

    int n = write(sock, to_send, PROTOCOL_STANDARD_MESSAGE_LENGTH);
    printf("Sending ENDCONN message\n");
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    free(to_send);
}

void send_DB_lastrow_as_JSON(int sock){
    int n;
    connectDB();
    struct resultStringArray result = getLastRow();

    send_init_connection_message(sock);  //Announcing start of message to the client

    json_object* jobj = create_custom_json(result);
    char* jobjstr = (char*)json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    printf("Received JSON string was: %s\n", jobjstr);
    char* header = "INCLUDE::";
    char* delimitor = "\n##ALBA##\n";

    //Now that we have the complete message, we proceed to adjust it to the protocol.
    char* token = strtok(jobjstr, "\n");
    int ii = 0;
    while (token != NULL) {
        int to_send_length = strlen(token) + strlen(header) + strlen(delimitor) + fillerArrayLength + 1;
        char* to_send = calloc(to_send_length, sizeof(char));
        printf("Part %i is: %s\n", ii, token);
        printf("With length %i bytes\n", to_send_length);

        //First, the header
        strcpy(to_send, header);
        //Next, the content of the message (the token in this case)
        strcat(to_send, token);
        //We put a delimitor, unlikely to appear in a normal connection
        strcat(to_send, delimitor);
        //Finally we concatenate the string with the filler string
        strcat(to_send, fillerArray);

        printf("Sending \"%s\" to the client\n", to_send);
        printf("Writing %i bytes on socket stream\n", PROTOCOL_STANDARD_MESSAGE_LENGTH);
        n = write(sock, to_send, PROTOCOL_STANDARD_MESSAGE_LENGTH);
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        //Next token
        token = strtok(NULL, "\n");
        free(to_send);
        ii++;
    }
    printf("Completed message in %i rounds!\n", ii);
    send_end_connection_message(sock);  //Announcing end of message to the client
}


/**
 * Here we start the reader thread, and wait for instructions / requests.
 * 
 */
void doprocessing(int sock) {
    pthread_t reading_thread;  //We need another thread to process the data sent by the client.
    pthread_create(&reading_thread, NULL, reading_thread_routine, &sock);
    
    printf("Starting processing\n");

    send_DB_lastrow_as_JSON(sock);


    while (1) {
        printf("Trapped\n");
        sleep(15);
        //Trapped here for now, until implementation of listening of client's commands.
        //Must not exit the proccess.
    }
}

int main(int argc, char* argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;
    int conn_number = 0;
    fillerArray = calloc(fillerArrayLength, sizeof(char));

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

    free(fillerArray);
}