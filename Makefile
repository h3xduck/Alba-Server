CC = gcc
CFLAGS  = -lmysqlclient -lz -ljson-c -pthread

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
all: server

# To create the object file server.o, we need the source
# files server.c, messages.h, messages.c, queue.h, queue.c, database.h, database.c, parser.h and parser.c:
#
server.o:  src/server.c include/database.h src/database.c include/parser.h src/parser.c include/queue.h src/queue.c src/messages.c include/messages.h
	$(CC) $(CFLAGS) -c server.c

# To create the object file database.o, we need the source files
# database.c and database.h:
#
database.o:  src/database.c include/database.h 
	$(CC) $(CFLAGS) -c database.c

# To create the object file parser.o, we need the source files
# parser.c and parser.h:
#
parser.o:  src/parser.c include/parser.h 
	$(CC) $(CFLAGS) -c parser.c

# To create the object file queue.o, we need the source files
# queue.c and queue.h:
#
queue.o:  rc/queue.c include/squeue.h 
	$(CC) $(CFLAGS) -c queue.c

# To create the object file messages.o, we need the source files
# messages.c and messages.h:
#
messages.o:  src/messages.c include/messages.h 
	$(CC) $(CFLAGS) -c messages.c

# To create the executable file count we need the object files
# server.o, database.o, parser.o, queue.o, messages.o
#
server:  server.o database.o parser.o queue.o messages.o
	$(CC) $(CFLAGS) -o server server.o database.o parser.o queue.o messages.o

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) count *.o *~