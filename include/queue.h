#ifndef QUEUE_H_  
#define QUEUE_H_

#include <stdbool.h>

struct message_manager_element* queue_top();

bool queue_isEmpty();

bool queue_isFull();

int queue_size();

void queue_enqueue(struct message_manager_element* data);

struct message_manager_element* queue_dequeue();

void queue_print_positions();


#endif