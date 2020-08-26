#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "database.h"
#include <json-c/json.h>

void signalHandler(){
    
}

json_object* create_custom_json(struct resultStringArray resultArray){
    json_object * jobj = json_object_new_object();
    json_object *jstring0 = json_object_new_string(resultArray.contentArray[0]);
    json_object *jstring1 = json_object_new_string(resultArray.contentArray[1]);
    json_object *jstring2 = json_object_new_string(resultArray.contentArray[2]);

    json_object_object_add(jobj,"Date", jstring0);
    json_object_object_add(jobj,"Content", jstring1);
    json_object_object_add(jobj,"User", jstring2);
    printf ("The json object created: %s\n",json_object_to_json_string(jobj));
    
    return jobj;
}

void doprocessing(int sock) {
    printf("Starting processing\n");

    int n;
    char buffer[256];
    bzero(buffer, 256);
    n = read(sock, buffer, 255);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Message received from client: %s\n", buffer);
    connectDB();
    struct resultStringArray result = getLastRow();

    json_object* jobj = create_custom_json(result);
    
    char* to_send;
    strcpy(to_send,"INCLUDE:");
    strcat(to_send,json_object_to_json_string_ext(jobj,JSON_C_TO_STRING_PRETTY));
    

    printf("Sending \"%s\" to the client\n", to_send);
    int length_to_send = strlen(to_send);
    printf("Writing %i bytes on socket stream\n", length_to_send);
    n = write(sock, to_send, length_to_send);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;
    int conn_number = 0;

    
    /* First call to socket() function */
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

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

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

        /* Create child process */
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(sockfd);
            doprocessing(newsockfd);
            printf("Closing connection...\n");
            exit(0);
        } else {
            close(newsockfd);
        }

    } /* end of while */
}