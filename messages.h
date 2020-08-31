#ifndef MESS_H_  
#define MESS_H_

typedef struct message_manager_element{
    struct parser_result* struct_element;
    int sock;

}message_manager_element;

void* message_manager_start(void* param);

#endif