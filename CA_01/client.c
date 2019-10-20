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
#define COMMAND_MAX_LENGTH 256
#define FILE_MAX_LENGTH 2048
#define MAXLINE 1024
#define HEARTBEAT_TIMEOUT 2
#define EOF_STR "EOF"

#define true 1
#define false 0


void print(char* buf) {
    write(1, buf, strlen(buf));
}


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
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("Error: set broadcast option failed!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

int createSocketFD(){
    // Creating socket file descriptor
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

struct sockaddr_in createServerAddress(int port){
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
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

// <HEARTBEAT ------------------------------

int getServerPort(int heartbeatPort){
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    sockfd = createSocketFD();
    servaddr = createServerAddress(heartbeatPort);
    setBroadcastOption(sockfd);
    bindSocketAndAddress(sockfd, servaddr);
    setTimeoutOption(sockfd, HEARTBEAT_TIMEOUT);

    socklen_t len = sizeof(servaddr);
    if(recvfrom(sockfd, (char *)buffer, sizeof(buffer),
                 MSG_WAITALL, (struct sockaddr *) &servaddr,
                 &len) < 0){
        perror("Server is dead!");
        return -1;
    }

    printf("Server is alive -> Listening Port: %s\n", buffer);

    close(sockfd);
    return atoi(buffer);
}

// HEARTBEAT ------------------------------/>

char* readFile(char* fileName){
    int fd, sz;
    char *c = (char *) calloc(100, sizeof(char));

    fd = open(fileName, O_RDONLY);
    if (fd < 0) { perror("r1"); exit(1); }

    sz = read(fd, c, 10);
    printf("called read(% d, c, 10).  returned that"
           " %d bytes  were read.\n", fd, sz);
    c[sz] = '\0';
//    printf("Those bytes are as follows: % s\n", c);
}

int uploadToServer(char* fileName, int serverPort){
    int sockfd;
    char buffer[MAXLINE];
    char* message = "Hello Server";
    struct sockaddr_in servaddr;

    int n, len;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print("socket creation failed");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr*)&servaddr,
                sizeof(servaddr)) < 0) {
        print("\nError : Connect Failed \n");
        return -1;
    }

    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("reading welcome failed"); return -1;}
    print(buffer);

    if(write(sockfd, UPLOAD_COMMAND, strlen(UPLOAD_COMMAND)) < 0) {print("upload command sending failed");return -1;}

    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("Upload command Accept failed");return -1;}

    if(write(sockfd, fileName, strlen(fileName)) < 0) {print("fileName sending failed");return -1;}
    bzero(buffer, sizeof(buffer));
    if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("fileName accept failed");return -1;}

    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {perror("r1"); return -1;}

    bzero(buffer, sizeof(buffer));
    while(read(fd, buffer, sizeof(buffer)) > 0){
        if(write(sockfd, buffer, sizeof(buffer)) < 0) {print("uploading file failed"); return -1;}
        bzero(buffer, sizeof(buffer));
        if(read(sockfd, buffer, sizeof(buffer)) < 0) {print("File part accept failed");return -1;}
        bzero(buffer, sizeof(buffer));
    }

    write(sockfd, EOF_STR, sizeof(EOF_STR));

    if(read(sockfd, buffer, sizeof(buffer)) < 0) {
        print("receive failed");
        return -1;
    }


//    strcpy(buffer, "Hello Server");
//    write(sockfd, buffer, sizeof(buffer));
//    printf("Message from server: ");
//    read(sockfd, buffer, sizeof(buffer));
    puts(buffer);
    close(sockfd);
}

int download(char* fileName, int heartbeatPort, int broadcastPort, int clientPort){

    return true;
}

int upload(char* fileName, int heartbeatPort, int broadcastPort, int clientPort){
    int serverPort = getServerPort(heartbeatPort);
    if(serverPort > 0){
        uploadToServer(fileName, serverPort);
    }else{

        return false;
    }
    return false;
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


int main(int argc, char** argv) {

    if(argc != 4){
        printf("ERROR: Client needs 3 arguments!\n");
        exit(-1);
    }
    int heartbeatPort = strtol(argv[1], NULL, 10);
    int broadcastPort = strtol(argv[2], NULL, 10);
    int clientPort = strtol(argv[3], NULL, 10);
    printf("Heartbeat Port: %d\nBroadcast Port: %d\nClient Port: %d\n", heartbeatPort, broadcastPort, clientPort);

    while(1){
        char *command, *fileName;
        getInput(&command, &fileName);


        if(strcmp(command, DOWNLOAD_COMMAND) == 0){
            printf("%sing %s...\n", command, fileName);
            download(fileName, heartbeatPort, broadcastPort, clientPort);
        }else if(strcmp(command, UPLOAD_COMMAND) == 0){
            printf("%sing %s...\n", command, fileName);
            upload(fileName, heartbeatPort, broadcastPort, clientPort);
        }else{
            printf("%s", "Please use mentioned commands!\n");
        }

    }


    return 0;
}
