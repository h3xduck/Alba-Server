#include "messages.h"

#include <json-c/json.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "database.h"
#include "parser.h"
#include "queue.h"
#include <sys/socket.h>
#include <errno.h>

int fillerArrayLength = PROTOCOL_STANDARD_MESSAGE_LENGTH * sizeof(char);
char* fillerArray;  //memory allocated at start.

extern pthread_mutex_t queue_mutex;
extern pthread_cond_t queue_non_empty;
extern pthread_cond_t queue_non_full;
extern pthread_mutex_t socket_stream_write_mutex;

/**
 * Generic function to send a message to the client.
 * @param sock: socket descriptor
 * @param header: header for the message
 * @param content: content to send
 */
void send_message(int sock, const char* header, const char* content) {

    char* separator = "\n##ALBA##\n";
    int totalLength = strlen(content) + strlen(header) + strlen(separator) + fillerArrayLength + 1;
    char* to_send = calloc(totalLength, sizeof(char));
    printf("Sending %i bytes\n", totalLength);

    //First, the header
    strcpy(to_send, header);
    //Next, the content of the message (the token in this case)
    strcat(to_send, content);
    //We put a delimitor, unlikely to appear in a normal connection
    strcat(to_send, separator);
    //Finally we concatenate the string with the filler string
    strcat(to_send, fillerArray);

    pthread_mutex_lock(&socket_stream_write_mutex);
    int res = send(sock, to_send, PROTOCOL_STANDARD_MESSAGE_LENGTH, MSG_DONTWAIT | MSG_NOSIGNAL);
    pthread_mutex_unlock(&socket_stream_write_mutex);
    
    if(res<-1){
        if(EWOULDBLOCK == errno || EAGAIN == errno) {
            perror("ERROR write would block");
        }else{
            perror("ERROR can't write to socket");
        }
    }
    //printf("Sending %s\n", to_send);

    free(to_send);
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

void send_init_connection_message(int sock) {
    printf("Sending STARTCONN message\n");
    send_message(sock, "STARTCONN::", "");
}

void send_end_connection_message(int sock) {
    printf("Sending ENDCONN message\n");
    send_message(sock, "ENDCONN::", "");
}

void send_DB_lastrow_as_JSON(int sock) {
    int n;
    connectDB();
    struct resultStringArray result = getLastRow();

    send_init_connection_message(sock);  //Announcing start of message to the client

    json_object* jobj = create_custom_json(result);
    char* jobjstr = (char*)json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    printf("Received JSON string was: %s\n", jobjstr);
    char* header = "INCLUDE::";

    //Now that we have the complete message, we proceed to adjust it to the protocol.
    char* token = strtok(jobjstr, "\n");
    while (token != NULL) {
        send_message(sock, header, token);
        token = strtok(NULL, "\n");
    }
    printf("Completed JSON message!");
    send_end_connection_message(sock);  //Announcing end of message to the client
}

/**
 * Sends a PONG message to the client connected to sock. 
 */
void sendPONG(int sock) {
    send_message(sock, "PONG::", "Hello client");
}

/**
 * Sends a PING message to the client connected to sock. 
 */
void sendPING(int sock) {
    send_message(sock, "PING::", "Hello client");
}

void* message_manager_start(void* params) {
    //First of all we parse the thread parameter.
    struct message_manager_param* param = (struct message_manager_param*) params;    
    int* FLAG_TERMINATE_THREAD = param->FLAG_TERMINATE_THREAD;
    int thread_id = param->thread_id;
    free(params);
    fillerArray = calloc(fillerArrayLength, sizeof(char));
    

    while (!*FLAG_TERMINATE_THREAD) {
        //Critical section
        pthread_mutex_lock(&queue_mutex);
        struct message_manager_element* element = queue_dequeue();
        pthread_mutex_unlock(&queue_mutex);
        
        printf("Writer thread %i dequeued a message\n", thread_id);
        //End of critical section

        //Now that we have the element, we evaluate it.
        switch (element->struct_element->result_code) {
            case -1:
                printf("Received ERROR\n");
                break;
            case 3:
                printf("Received PING\n");
                sendPONG(element->sock);
                break;
            case 4:
                printf("Received PONG\n");
                break;
            case 5:
                printf("Client request arrived\n");
                //TODO: This is temporal and just a test. We need a parser for REQUESTS too.
                send_DB_lastrow_as_JSON(element->sock);
                break;
            case 300:
                printf("Client disconnects\n");
            default:
                break;
        }

        free(element->struct_element);
        free(element);
    }

    printf("The termination flag was set to %i, thread %i exits\n", *FLAG_TERMINATE_THREAD, thread_id);
    pthread_exit(0);
    

    //TODO solve this: free(fillerArray);
}
