# ALBA - Server
This repository contains the source code of the Alba Server, which connects to the ALBA mobile app. For more information about what is ALBA and the app, have a look at [this other repo](https://github.com/marsan27/Alba/). Please note that both ALBA and ALBA-Server are currently on development and therefore they are **potentially unstable**.

The server project is written in C and it is POSIX compliant. It supports multiple simultaneous clients through a combination of multithreading and multiprocessing. DB management is handled with MySQL, and therefore **you will need one MySQL local installation to run this software**. Until the DB configuration script is ready, please check below the configurations to be made.

## Features overview 
- [x] Build a SocketServer and manage multiple clients on separate processes.
- [x] Construct a PING-PONG client-server system to manage disconnections on both sides.
- [x] Send and receive messages from clients using TCP sockets, defining [the ALBAProtocol](https://github.com/marsan27/Alba/blob/master/ALBAProtocol.md).
- [x] Construct a process-dependant circular queue to store requests and tasks for each client
- [x] Create a thread pool for each process, which retrieve elements from the process queue, ensuring minimum response times and maximizing performance.
- [x] Use mutex and conditional variables to avoid race conditions and ensure the critical sections are thread-safe. Also, implemented non-blocking socket read/write operations, maximizing resposiveness.
- [x] Implemented MySQL DB operations, which are always called from background threads, avoiding unnecessary wait times.
- [x] Constructed thread-safe program start and halt, without asynchronous cancellations.
- [x] Created a Parser in charge of translating socket messages into different codes according to the ALBAProtocol.
- [x] Send long messages by parts using JSON format and correctly announcing it comes divided into small chunks.

## Roadmap
- [ ] Implement more MySQL operations and tables according to different requests the client may send.
- [ ] Define more operations for the client and server.
- [ ] Accept different arguments at the program start, allowing for different modes.
- [ ] Create a basic GUI (probably with QT5).
- [ ] Keep ideas flowing.

## Build and run
```shell
##Downloading the program
git clone git://github.com/marsan27/alba-server.git

cd alba-server

##Build
make all

##Run
./server
```
Also, you will need to setup a MySQL DB with the following parameters:
* Database name = "VERNOM"
* GRANT acces to user = "Vernom" and password = "vernomPassword";

Note: Hardcoded passwords are not secure. This will change in the future.

## Disclaimer
This is a personal side-project and by no means it is supposed to come with any warranty, it is offered AS-IS and I am not responsible of any harm derived from its use.

## License
This project is released under the GPL v3 license. See [LICENSE](https://github.com/marsan27/Alba-Server/blob/master/LICENSE).
