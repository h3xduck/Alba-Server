#ifndef QUEUE_H_  
#define QUEUE_H_

struct parser_result top();

bool isEmpty();

bool isFull();

int size();

void enqueue(struct parser_result data);

struct parser_result dequeue();


#endif