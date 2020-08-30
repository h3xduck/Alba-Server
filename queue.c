#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define MAX 10

struct parser_result dataArray[MAX];
int front = 0;
int rear = -1;
int itemCount = 0;

struct parser_result top() {
    return dataArray[front];
}

bool isEmpty() {
    return itemCount == 0;
}

bool isFull() {
    return itemCount == MAX;
}

int size() {
    return itemCount;
}

void enqueue(struct parser_result data) {
    if (!isFull()) {
        if (rear == MAX - 1) {
            rear = -1;
        }
        dataArray[++rear] = data;
        itemCount++;
    }
}

struct parser_result dequeue() {
    struct parser_result data = dataArray[front++];
    if (front == MAX) {
        front = 0;
    }
    itemCount--;
    return data;
}