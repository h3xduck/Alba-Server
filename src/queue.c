#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include <pthread.h>
#include "../include/messages.h"

extern pthread_mutex_t queue_mutex;  
extern pthread_cond_t queue_non_empty;
extern pthread_cond_t queue_non_full;

#define MAX_ENQUEUED_REQUESTS 10

struct message_manager_element* dataArray[MAX_ENQUEUED_REQUESTS];
int front = 0;
int rear = -1;
int itemCount = 0;

struct message_manager_element queue_top() {
    return *dataArray[front];
}

bool queue_isEmpty() {
    return itemCount == 0;
}

bool queue_isFull() {
    return itemCount == MAX_ENQUEUED_REQUESTS;
}

int queue_size() {
    return itemCount;
}

void queue_enqueue(struct message_manager_element* data) {
    while(queue_isFull()){
        //The server reader must wait for some thread to dequeue a task.
        printf("Blocked enqueueing\n");
        pthread_cond_wait(&queue_non_full, &queue_mutex);
        
    }
    if (rear == MAX_ENQUEUED_REQUESTS - 1) {
        rear = -1;
    }
    dataArray[++rear] = data;
    itemCount++;
    printf("Enqueued element on position %i. Remaining elements: %i\n", rear, itemCount);
    pthread_cond_signal(&queue_non_empty);
}

struct message_manager_element* queue_dequeue() {
     while(queue_isEmpty()){
        //Waiting until the server reader enqueues some request.
        printf("Blocked dequeueing\n");
        pthread_cond_wait(&queue_non_empty, &queue_mutex);
    }
    printf("Dequeued element on position %i. Remaining elements: %i\n", front, itemCount-1);
    struct message_manager_element* data = dataArray[front++];
    if (front == MAX_ENQUEUED_REQUESTS) {
        front = 0;
    }
    itemCount--;
    pthread_cond_signal(&queue_non_full);
    return data;
    
}

void queue_print_positions(){

    printf("QUEUE:\nFront = %i\nRear = %i\n", front, rear);
}