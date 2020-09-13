# ALBAProtocol

This protocol defines how messages are sent between clients and server using TCP.

## Message structure
Every message sent/received by both the server and clients is 1024 bytes long. Messages consist of:
* A HEADER of variable length:
  
| HEADER | Sent by client? | Sent by server? |                           MEANING                          |
|:----------:|:-------------------:|:-------------------:|:--------------------------------------------------------------:|
|   INCLUDE  |         Yes         |         Yes         |                    New element for local DB                    |
|    ERROR   |         Yes         |         Yes         |                         Error reported                         |
|    INFO    |         Yes         |         Yes         |                           Information                          |
|    PING    |         Yes         |         Yes         |         A PING which must be answered before a timeout         |
|    PONG    |         Yes         |         Yes         |                       Response to a PING                       |
|  STARTCONN |          No         |         Yes         | Start of long connection divided into parts. Ends with ENDCONN |
|   ENDCONN  |          No         |         Yes         |                     End of long connection                     |
|   REQUEST  |         Yes         |          No         |                        Request some info                       |
|   DISCONN  |         Yes         |          NO         |                 Disconnection message by client                |


* Then, a header separator, namely ```::```.
* A buffer of variable length, containing some message or code to be processed depending on the header.
* The Protocol Separator, namely ```\n##ALBA##\n```.
* An array of bytes initialized to 0, until the message is 1024 bytes.

## Other considerations
* Any variation of the above is considered an invalid message.