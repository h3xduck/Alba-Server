#include "parser.h"
#include <string.h>
#include <stdbool.h>

char** valid_headers = {"d","a"};

struct parser_result protocol_parse(char* buffer){
    struct parser_result result;
}

bool is_valid_protocol_message(char* buffer){
    char* token = strtok(buffer, "::");

}