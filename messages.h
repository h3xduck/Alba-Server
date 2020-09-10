#ifndef MESS_H_  
#define MESS_H_

typedef struct message_manager_element{
    struct parser_result* struct_element;
    int sock;

}message_manager_element;

typedef struct message_manager_param{
    int thread_id;
    int* FLAG_TERMINATE_THREAD;  //This flag changes from the reader thread for all the writer threads.

}message_manager_param;

void* message_manager_start(void* param);

#endif