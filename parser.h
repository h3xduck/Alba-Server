#ifndef PARSER_H_  
#define PARSER_H_

struct parser_result{
    /**
     * Code indicating result of parsing the message:
     * 0 if it is a new message to include in DB (they always come in JSON format)
     * 1 if it is an error sent by the client
     * 2 if it is some info sent by the client, not needed to be included in the DB
     * 3 if it is a PING
     * 4 if it is a PONG
     * 5 if it is a REQUEST (from the client)
     * -1 if error (parser could not parse)
     * 100 if communication block starts
     * 200 if communication block ends
     * 300 if the client disconnects
     */ 
    int result_code; 

    /**
     * Buffer which stores info, when needed, depending on the result code:
     * Allocated char* when result_code is 0, 1 or 2.
     * NULL for other codes.
     */
    char* result_buffer;
};

struct parser_result* protocol_parse(char* buffer);


#endif