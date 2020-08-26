CC = gcc
CFLAGS  = -lmysqlclient -lz -ljson-c -pthread

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
all: server

# To create the object file server.o, we need the source
# files server.c, database.h, and database.c:
#
server.o:  server.c database.h database.c 
	$(CC) $(CFLAGS) -c server.c

# To create the object file database.o, we need the source files
# database.c and database.h:
#
database.o:  database.c database.h 
	$(CC) $(CFLAGS) -c database.c

# To create the executable file count we need the object files
# server.o, database.o
#
server:  server.o database.o
	$(CC) $(CFLAGS) -o server server.o database.o

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) count *.o *~