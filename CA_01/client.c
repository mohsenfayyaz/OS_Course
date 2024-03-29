//
// Created by mohsenfayyaz on 10/19/19.
//

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

#define UPLOAD_COMMAND "upload"
#define DOWNLOAD_COMMAND "download"
#define FILE_REQUEST_PREFIX "I_WANT "
#define FILE_SHARE_PREFIX "I_HAVE "

#define SERVER_HAS_FILE "SERVER_HAS_FILE"
#define SERVER_NOT_FOUND_FILE "SERVER_DOES_NOT_HAVE_FILE"

#define COMMAND_MAX_LENGTH 1024
#define FILE_MAX_LENGTH 2048
#define MAXLINE 1024
#define HEARTBEAT_TIMEOUT 2
#define EOF_STR "EOF"

#define TRUE   1
#define FALSE  0

#define EMPTY ""
char* globalFileNameToDownload = EMPTY;
int globalBroadcastPort = -1;
int globalBroadcastSocketFD;
struct sockaddr_in globalBroadcastAddress;
int globalBroadcastFileNameCounter = 0;

#define BROADCAST_FILENAME_INTERVAL 6
#define MAX_FILENAME_BROADCAST_TIMES 5

char* globalClientPortString;

void print(char* buf) {
    write(1, buf, strlen(buf));
}

int max(int x, int y){
    if (x > y)
        return x;
    else
        return y;
}


struct sockaddr_in createServerAddress(int port){
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY
    return servaddr;
}

struct sockaddr_in createBroadcastAddress(int port){
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //INADDR_ANY
    return servaddr;
}

void bindSocketAndAddress(int sockfd, struct sockaddr_in servaddr){
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

// 1)
int createUDPSocketFD(){
    // Creating socket file descriptor
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int createTCPSocketFD(){
    // Creating socket file descriptor
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// 2)
void bindSocketToPort(int sockfd, int port){
    bindSocketAndAddress(sockfd, createServerAddress(port));
}

// 3)
void setTimeoutOption(int rcv_sock, int seconds){
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(rcv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
        close(rcv_sock);
        exit(EXIT_FAILURE);
    }
}

void setBroadcastOption(int sockfd){
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0){
        perror("Error: set broadcast option failed!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("Error: set broadcast option failed!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Error: set broadcast option failed!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void setReusableOption(int sockfd){
    int opt = TRUE;
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
}

// <HEARTBEAT ------------------------------
int getServerPort(int heartbeatPort){
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    sockfd = createUDPSocketFD();
    servaddr = createBroadcastAddress(heartbeatPort);

    setBroadcastOption(sockfd);
    bindSocketAndAddress(sockfd, servaddr);
    setTimeoutOption(sockfd, HEARTBEAT_TIMEOUT);

    socklen_t len = sizeof(servaddr);
    int valread;
    if(valread = recvfrom(sockfd, (char *)buffer, sizeof(buffer),
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len) < 0){
        perror("Server is dead!");
        return -1;
    }

    // printf("Server is alive -> Listening Port: %s\n", buffer);
    print("Server is alive\n");
    close(sockfd);
    return atoi(buffer);
}


// HEARTBEAT ------------------------------/>

int uploadToServer(char* fileName, int serverPort){
    int sockfd;
    char buffer[MAXLINE];
    char* message = "Hello Server";
    struct sockaddr_in servaddr;

    int n, len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print("socket creation failed");
        return FALSE;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {perror("r1"); close(sockfd); return FALSE;}

    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("reading welcome failed"); return FALSE;}
    print(buffer);

    if(write(sockfd, UPLOAD_COMMAND, strlen(UPLOAD_COMMAND)) < 0) {print("upload command sending failed");return FALSE;}

    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("Upload command Accept failed");return FALSE;}

    if(write(sockfd, fileName, strlen(fileName)) < 0) {print("fileName sending failed");return FALSE;}
    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("fileName accept failed");return FALSE;}

    bzero(buffer, sizeof(buffer));
    while(read(fd, buffer, sizeof(buffer)) > 0){
        if(write(sockfd, buffer, sizeof(buffer)) < 0) {print("uploading file failed"); return FALSE;}
        bzero(buffer, sizeof(buffer));
        if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("File part accept failed");return FALSE;}
        bzero(buffer, sizeof(buffer));
    }

    write(sockfd, EOF_STR, sizeof(EOF_STR));

    if(read(sockfd, buffer, sizeof(buffer)) < 0) {
        print("receive failed");
        return FALSE;
    }


//    strcpy(buffer, "Hello Server");
//    write(sockfd, buffer, sizeof(buffer));
//    printf("Message from server: ");
//    read(sockfd, buffer, sizeof(buffer));
    puts(buffer);
    close(sockfd);
    return TRUE;
}

int downloadFromServer(char* fileName, int serverPort){
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    int n, len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print("socket creation failed");
        return FALSE;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {
        print("\nError : Connect Failed \n");
        return FALSE;
    }

    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("reading welcome failed"); return FALSE;}
    print(buffer);

    if(write(sockfd, DOWNLOAD_COMMAND, strlen(DOWNLOAD_COMMAND)) < 0) {print("download command sending failed");return FALSE;}
    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("download command Accept failed");return FALSE;}

    if(write(sockfd, fileName, strlen(fileName)) < 0) {print("fileName sending failed");return FALSE;}
    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("fileName accept failed");return FALSE;} // SERVER_HAS... or SERVER_DOES_NOT...

    if(strcmp(buffer, SERVER_HAS_FILE) == 0){ //IF SERVER HAS FILE
        unlink(fileName);
        int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd < 0) {perror("r1"); close(sockfd); exit(1);}

        if(write(sockfd, "Send Please!", strlen("Send Please!")) < 0) {print("Send please failed");return FALSE;}

        bzero(buffer, sizeof(buffer));
        while (read(sockfd, buffer, sizeof(buffer)) > 0){
            // printf("\n%s\n", buffer);
            print(buffer); print("\n");
            if(strcmp(buffer, EOF_STR) == 0){
                break;
            }
            write(fd, buffer, strlen(buffer));
            write(sockfd, "File part Accepted", sizeof("File part Accepted"));
            bzero(buffer, sizeof(buffer));
        }

        close(fd);
        close(sockfd);
        return TRUE;

    }
    

    close(sockfd);
    return FALSE;
}

void broadcast(char* buffer){
    if (sendto(globalBroadcastSocketFD, buffer, strlen(buffer),
           /*MSG_CONFIRM*/ 0, (const struct sockaddr *) &globalBroadcastAddress,
           sizeof(globalBroadcastAddress)) != strlen(buffer)){

        perror("broadcasting file name failed");
        exit(EXIT_FAILURE);
    }

    print("sent to Broadcast: |");
    print(buffer);
    print("|\n");
}

void broadCastFileNameToDownload(){
    if(globalFileNameToDownload == EMPTY){
        globalBroadcastFileNameCounter = 0;
        return;
    }else{
        if(globalBroadcastFileNameCounter > MAX_FILENAME_BROADCAST_TIMES){
            print("NO PEER HAD THE FILE!\n");
            globalBroadcastFileNameCounter = 0;
            globalFileNameToDownload = EMPTY;
            return;
        }else{
            globalBroadcastFileNameCounter++;
            char buffer[COMMAND_MAX_LENGTH];
            bzero(buffer, sizeof(buffer));
            strcat(buffer, DOWNLOAD_COMMAND);
            strcat(buffer, " ");
            strcat(buffer, globalFileNameToDownload);
            broadcast(buffer);
            // alarm(BROADCAST_FILENAME_INTERVAL);
        }
    }
    
}

void downloadFromPeers(char* fileName){
    globalFileNameToDownload = fileName;
    // signal(SIGALRM, broadCastFileNameToDownload);
    broadCastFileNameToDownload();
}

int download(char* fileName, int heartbeatPort, int broadcastPort, int clientPort){
    int serverPort = getServerPort(heartbeatPort);
    if(serverPort > 0){
        if(downloadFromServer(fileName, serverPort) == FALSE){
            print("Server Failed Finding File\n");
            print("Trying Peer2Peer...\n");
            downloadFromPeers(fileName);
        }else{
            return TRUE;
        }
    }else{
        print("Trying Peer2Peer...\n");
        downloadFromPeers(fileName);
        return FALSE;
    }
    return FALSE;
}

int upload(char* fileName, int heartbeatPort, int broadcastPort, int clientPort){
    int serverPort = getServerPort(heartbeatPort);
    if(serverPort > 0){
        return uploadToServer(fileName, serverPort);
    }
    return FALSE;
}

void getInput(char** command, char** fileName){
    (*command) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    (*fileName) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    print("-----------------------------\n");
    print("-> Please write your command (upload/download):\n");
    read(0, (*command), COMMAND_MAX_LENGTH);
    print("-> Please write file name (FILE_NAME):\n");
    read(0, (*fileName), COMMAND_MAX_LENGTH);

    (*command)[strlen((*command))-1] = '\0';
    (*fileName)[strlen((*fileName))-1] = '\0';
}

void parseInput(char** command, char**fileName, char* line){
    (*command) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    (*fileName) = (char*) malloc(COMMAND_MAX_LENGTH * sizeof(char));
    int commandOrFile = 0;
    int spacePos;
    for(int i=0; i<strlen(line); i++){
        if(line[i] == ' '){
            commandOrFile = 1;
            spacePos = i+1;
            (*command)[i] = '\0';
        }else{
            if(commandOrFile == 0){
                (*command)[i] = line[i];
            }else{
                (*fileName)[i - spacePos] = line[i];
            }
        }
    }
    (*fileName)[strlen(line) - spacePos] = '\0';
}



int handleDownloadFromServer(int connfd){ //GIVING OTHER PEERS WHAT YOU HAVE
    int valread;
    write(connfd, "Download Accepted", sizeof("Download Accepted"));

    char buffer[MAXLINE];
    char fileName[MAXLINE];

    bzero(fileName, sizeof(buffer));
    if(valread = read(connfd, fileName, sizeof(fileName)) < 0) {return -1;}
    
    // printf("Request for downloading %s\n", fileName);
    print("Request for downloading "); print(fileName); print("\n");

    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        write(connfd, SERVER_NOT_FOUND_FILE, sizeof(SERVER_NOT_FOUND_FILE));
        close(connfd);
        return FALSE;
    }

    write(connfd, SERVER_HAS_FILE, sizeof(SERVER_HAS_FILE));
    if(read(connfd, buffer, sizeof(buffer)) < 0) {print("RECIVEING SEND PLEASE FAILED");return FALSE;}

    bzero(buffer, sizeof(buffer));
    while(read(fd, buffer, sizeof(buffer)) > 0){
        if(write(connfd, buffer, sizeof(buffer)) < 0) {print("uploading file failed"); return FALSE;}
        bzero(buffer, sizeof(buffer));
        if(read(connfd, buffer, sizeof(buffer)) < 0) {print("File part accept failed");return FALSE;}
        bzero(buffer, sizeof(buffer));
    }

    write(connfd, EOF_STR, sizeof(EOF_STR));

    // printf("Someone downloaded %s\n", fileName);
    print("Someone downloaded "); print(fileName); print("\n");

    close(fd);
}







void reOpenBroadcastSocket(){
    //------------------<CREATE BROADCAST SOCKET FOR UDP PEER2PEER----
    close(globalBroadcastSocketFD);
    int broadcastSocket = createUDPSocketFD();
    setBroadcastOption(broadcastSocket);
    struct sockaddr_in broadcastAddress = createBroadcastAddress(globalBroadcastPort);
    if (bind(broadcastSocket, (struct sockaddr *)&broadcastAddress, sizeof(broadcastAddress))<0) {perror("Broadcast bind failed"); exit(EXIT_FAILURE);}
    // puts("Waiting for interrupts on broadcast Port ...");
    globalBroadcastSocketFD = broadcastSocket;
    globalBroadcastAddress = broadcastAddress;
    //------------------CREATE BROADCAST SOCKET FOR UDP PEER2PEER/>----
}




int runClient(int clientPort, int heartbeatPort, int broadcastPort){
    int opt = TRUE;
    int addrlen , new_socket , client_socket[30] ,max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "PEER SERVER: WELCOME TO SERVER \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //------------------<CREATE CLIENT SOCKET FOR TCP FILE TRANSFER----
    //create a master socket
    int masterSocket = createTCPSocketFD();
    //set master socket to allow multiple connections ,
    setReusableOption(masterSocket);
    //type of socket created
    struct sockaddr_in address = createServerAddress(clientPort);
    //bind the socket to localhost port serverPort
    if (bind(masterSocket, (struct sockaddr *)&address, sizeof(address))<0) {perror("Client bind failed"); exit(EXIT_FAILURE);}
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(masterSocket, 3) < 0) {perror("listen"); exit(EXIT_FAILURE);}
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections on clientPort ...");
    //------------------CREATE CLIENT SOCKET FOR TCP FILE TRANSFER/>----



    //------------------<CREATE BROADCAST SOCKET FOR UDP PEER2PEER----
    // close(globalBroadcastSocketFD);
    int broadcastSocket = createUDPSocketFD();
    setBroadcastOption(broadcastSocket);
    struct sockaddr_in broadcastAddress = createBroadcastAddress(broadcastPort);
    if (bind(broadcastSocket, (struct sockaddr *)&broadcastAddress, sizeof(broadcastAddress))<0) {perror("Broadcast bind failed"); exit(EXIT_FAILURE);}
    // puts("Waiting for interrupts on broadcast Port ...");
    globalBroadcastPort = broadcastPort;
    globalBroadcastSocketFD = broadcastSocket;
    globalBroadcastAddress = broadcastAddress;
    //------------------CREATE BROADCAST SOCKET FOR UDP PEER2PEER/>----

    print("-----------------------------\n");
    print("-> Please write your command (upload/download FileName):\n");
    
    clock_t start, end;
    start = clock();
    while(TRUE)
    {
        end = clock();
        // int msec = difference * 1000 / CLOCKS_PER_SEC;
        // printf("|%ld|", clock());
        // if(msec > 1000){
        //     print("aaa");
        //     before = clock();
        // }
        if(end - start > BROADCAST_FILENAME_INTERVAL*100){
            // print("sss");
            // printf("start = %d, end = %d\n", start, end);
            reOpenBroadcastSocket();
            broadCastFileNameToDownload();
            start = clock();
        }
        
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(globalBroadcastSocketFD, &readfds); /* Add BROADCAST socket to descriptor vector */
        FD_SET(STDIN_FILENO, &readfds); /* Add keyboard to descriptor vector */
        FD_SET(masterSocket, &readfds); /* Add CLIENT socket to descriptor vector */
        max_sd = max(masterSocket, globalBroadcastSocketFD);

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        // print("->a ");
        struct timeval tv;
        tv.tv_sec = (long)1;
        tv.tv_usec = 0;
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);
        // print("->s \n");
        if ((activity < 0) && (errno!=EINTR))
        {
            print("select error");
        }
        while (errno==EINTR && FD_ISSET(masterSocket, &readfds)) //Alarm Interrupt
        {
            print("ALARM INTERRUPT!");
            activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv);
        }
        
        if (FD_ISSET(masterSocket, &readfds))
        {
            print("MasterSocket Interrupt\n");
            if ((new_socket = accept(masterSocket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            print("aaa\n");
            //inform user of socket number - used in send and receive commands
            // printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
            //         (address.sin_port));
            print("New connection\n");

            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    // printf("Adding to list of sockets as %d\n" , i);
                    print("Adding to list of sockets\n");
                    break;
                }
            }
        }

        if (FD_ISSET(globalBroadcastSocketFD, &readfds)){
            print("Broadcast Interrupt: ");
            char *command, *fileName;
            // char line[COMMAND_MAX_LENGTH];
            // valread = read(broadcastSocket, line, COMMAND_MAX_LENGTH);

            // int sockfd;
            char buffer[MAXLINE];
            // struct sockaddr_in servaddr;
            // sockfd = createUDPSocketFD();
            // servaddr = createServerAddress(globalBroadcastPort);
            // setBroadcastOption(sockfd);
            // bindSocketAndAddress(sockfd, servaddr);
            // setTimeoutOption(sockfd, 2);

            // socklen_t len = sizeof(servaddr);
            int valread;
            bzero(buffer, sizeof(buffer));
            // if(valread = recvfrom(sockfd, (char *)buffer, MAXLINE,
            //             MSG_WAITALL, (struct sockaddr *) &servaddr,
            //             &len) < 0){
            //     perror("Failed to read Broadcast");
            //     return -1;
            // }
            socklen_t len = sizeof(globalBroadcastAddress);
            if(valread = recvfrom(globalBroadcastSocketFD, (char *)buffer, MAXLINE,
                        MSG_WAITALL, (struct sockaddr *) &globalBroadcastAddress,
                        &len) < 0){
                perror("Failed to read Broadcast");
                return -1;
            }
            

            // valread = read(STDIN_FILENO, buffer, COMMAND_MAX_LENGTH);
            // buffer[valread-1] = '\0'; //REMOVE \n

            // close(sockfd);

            // buffer[valread] = '\0';
            print("|"); print(buffer); print("|\n");
            parseInput(&command, &fileName, buffer);
            // print(fileName);
            if(strcmp(command, DOWNLOAD_COMMAND) == 0){ //DOWNLOAD FILENAME on BROADCAST
                if(strcmp(globalFileNameToDownload, fileName) == 0){
                    // reOpenBroadcastSocket();
                    print("It's ME! :)\n");
                    continue;
                }
                int fd = open(fileName, O_RDONLY);
                if (fd > 0) { //WE HAVE THE FILE
                    char buffer[COMMAND_MAX_LENGTH];
                    bzero(buffer, sizeof(buffer));
                    strcat(buffer, globalClientPortString);
                    strcat(buffer, " ");
                    strcat(buffer, fileName);
                    reOpenBroadcastSocket();
                    broadcast(buffer);
                }else{
                    perror("File not found");
                }
            }else{
                if(strcmp(fileName, globalFileNameToDownload) == 0){ // PORT FILENAME on BROADCAST
                    if(downloadFromServer(fileName, strtol(command, NULL, 10)) == TRUE){
                        globalFileNameToDownload = EMPTY;
                        print("File downloaded successfully from peers\n");
                    }
                }
            }
            // reOpenBroadcastSocket();
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)){
            print("INPUT Interrupt\n");
            char *command, *fileName;
            char line[COMMAND_MAX_LENGTH];
            valread = read(STDIN_FILENO, line, COMMAND_MAX_LENGTH);
            line[valread-1] = '\0'; //REMOVE \n
            parseInput(&command, &fileName, line);

            if(strcmp(command, DOWNLOAD_COMMAND) == 0){
                // printf("%sing %s...\n", command, fileName);
                print(command); print("ing "); print(fileName); print("...\n");
                if(download(fileName, heartbeatPort, broadcastPort, clientPort) == TRUE){
                    print("File downloaded successfully \n");
                }else{
                    print("Peer2Peer \n");
                }

            }else if(strcmp(command, UPLOAD_COMMAND) == 0){
                print(command); print("ing "); print(fileName); print("...\n");
                if(upload(fileName, heartbeatPort, broadcastPort, clientPort) == TRUE){
                  print("File uploaded successfully \n");
                }else{
                    print("File uploading failed \n");
                }
            }else{
                print("Please use mentioned commands!\n");
            }
            print("-----------------------------\n");
            print("-> Please write your command (upload/download FileName):\n");
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if(sd == STDIN_FILENO || sd == globalBroadcastPort) continue;
            if (FD_ISSET( sd , &readfds))
            {
                print("Client Interrupt\n");
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    // printf("Host disconnected , ip %s , port %d \n" ,
                    //        inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    print("Host disconnected\n");

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                    //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end
                    //of the data read
                    buffer[valread] = '\0';
                    // printf("->%s\n", buffer);
                    print(buffer); print("\n");
                    if(strncmp(buffer, DOWNLOAD_COMMAND, strlen(DOWNLOAD_COMMAND)) == 0){
                        handleDownloadFromServer(sd);
                    }
                }
            }
        }
    }

    return 0;
}
















int main(int argc, char** argv) {

    if(argc != 4){
        print("ERROR: Client needs 3 arguments!\n");
        exit(-1);
    }
    int heartbeatPort = strtol(argv[1], NULL, 10);
    int broadcastPort = strtol(argv[2], NULL, 10);
    int clientPort = strtol(argv[3], NULL, 10);
    globalClientPortString = argv[3];
    // printf("Heartbeat Port: %d\nBroadcast Port: %d\nClient Port: %d\n", heartbeatPort, broadcastPort, clientPort);

    runClient(clientPort, heartbeatPort, broadcastPort);

    // while(1){
    //     char *command, *fileName;
    //     getInput(&command, &fileName);


    //     if(strcmp(command, DOWNLOAD_COMMAND) == 0){
    //         printf("%sing %s...\n", command, fileName);
    //         download(fileName, heartbeatPort, broadcastPort, clientPort);
    //     }else if(strcmp(command, UPLOAD_COMMAND) == 0){
    //         printf("%sing %s...\n", command, fileName);
    //         upload(fileName, heartbeatPort, broadcastPort, clientPort);
    //     }else{
    //         printf("%s", "Please use mentioned commands!\n");
    //     }
    // }


    return 0;
}
