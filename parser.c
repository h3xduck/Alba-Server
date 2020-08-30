#include "parser.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define HEADER_SEPARATOR "::"
#define PROTOCOL_SEPARATOR "\n##ALBA##\n"

char* valid_protocol_headers[] = {"INCLUDE", "ERROR", "INFO", "PING", "PONG", "STARTCONN", "ENDCONN"};

struct parser_result protocol_parse(char* buffer){
    struct parser_result result;
    result.result_buffer = malloc(sizeof(char)*1024);
    char* content;
    
    char* token = strtok(buffer, HEADER_SEPARATOR);
    if(token!=NULL){
        char* content = strtok(NULL, PROTOCOL_SEPARATOR);
        if(content==NULL){
            perror("Invalid content received, package invalid.");
            result.result_code = -1;
            result.result_buffer = NULL;
            return result;
        }
        if(strcmp(token, valid_protocol_headers[0])){
            result.result_code = 0;
            result.result_buffer = content;
        }else if(strcmp(token, valid_protocol_headers[1])){
            result.result_code = 1;
            result.result_buffer = content;
        }else if(strcmp(token, valid_protocol_headers[2])){
            result.result_code = 2;
            result.result_buffer = content;
        }else if(strcmp(token, valid_protocol_headers[3])){
            result.result_code = 3;
            result.result_buffer = NULL;
        }else if(strcmp(token, valid_protocol_headers[4])){
            result.result_code = 4;
            result.result_buffer = NULL;
        }else if(strcmp(token, valid_protocol_headers[5])){
            result.result_code = 100;
            result.result_buffer = NULL;
        }else if(strcmp(token, valid_protocol_headers[6])){
            result.result_code = 200;
            result.result_buffer = NULL;
        }else{
            //Invalid header
            perror("Invalid header received, package invalid");
            result.result_code = -1;
            result.result_buffer = NULL;
        }
    }else{
        //Unknown header
        perror("Header not recognized, package invalid");
        result.result_code = -1;
        result.result_buffer = NULL;
    }
    return result;
}
