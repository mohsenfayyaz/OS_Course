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

#define MAX 80
#define PORT 8080
#define MAXLINE 1024
#define SA struct sockaddr
#define SERVER_PORT "8080"

#define UPLOAD_COMMAND "upload"
#define DOWNLOAD_COMMAND "download"

#define SERVER_HAS_FILE "SERVER_HAS_FILE"
#define SERVER_NOT_FOUND_FILE "SERVER_DOES_NOT_HAVE_FILE"

#define EOF_STR "EOF"
#define TRUE   1
#define FALSE  0

int globalHeartbeatPort;
int globalHeartbeatSocketFD;
struct sockaddr_in globalHeartbeatAddress;

void print(char* buf) {
    write(1, buf, strlen(buf));
}

// <HEARTBEAT ------------------------------
void handleHeartbeatSignal(){
    if (sendto(globalHeartbeatSocketFD, SERVER_PORT, strl ¡¨sþen(SERVER_PORT),
           MSG_CONFIRM, (const struct sockaddr *) &globalHeartbeatAddress,
           sizeof(globalHeartbeatAddress)) != strlen(SERVER_PORT)){

        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    alarm(1);
}

void startHeartbeat(int heartbeatPort){
    int sockfd;
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0){
        close(sockfd);
        perror("socket options failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        close(sockfd);
        perror("socket options failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        close(sockfd);
        perror("socket options failed");
   ¡¨sþ      exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_BROADCAST;
    servaddr.sin_port = htons(heartbeatPort);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    globalHeartbeatPort = heartbeatPort;
    globalHeartbeatSocketFD = sockfd;
    globalHeartbeatAddress = servaddr;
    signal(SIGALRM, handleHeartbeatSignal);
    alarm(1);

}

// HEARTBEAT ------------------------------/>

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

int handleDownloadFromServer(int connfd){
    int valread;
    write(connfd, "Download Accepted", sizeof("Download Accepted"));

    char buffer[MAXLINE];
    char fileName[MAXLINE];

    bzero(fileName, sizeof(buffer));
    if(valread = rea ¡¨sþd(connfd, fileName, sizeof(fileName)) < 0) {return -1;}
    
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

    print("Someone downloaded "); print(fileName); print("\n");

    close(fd);
}

int handleUploadToServer(int connfd){
   ¡¨sþ  int valread;
    write(connfd, "Upload Accepted", sizeof("Upload Accepted"));

    char buffer[MAXLINE];
    char fileName[MAXLINE];

    bzero(fileName, sizeof(buffer));
    if(valread = read(connfd, fileName, sizeof(fileName)) < 0) {return -1;}
    write(connfd, "FileName Accepted", sizeof("FileName Accepted"));
    print("Writing on "); print(fileName); print("\n");

    unlink(fileName);
    int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {perror("r1");exit(1);}


    while (valread = read(connfd, buffer, sizeof(buffer)) > 0){
        print(buffer); print("\n");
        if(strcmp(buffer, EOF_STR) == 0){
            break;
        }

        write(fd, buffer, strlen(buffer));
        write(connfd, "File part Accepted", sizeof("File part Accepted"));
        bzero(buffer, sizeof(buffer));
    }

    close(fd);
    write(connfd, "Server: Done", sizeof("Server: Done"));

}

int runServer(int serverPort){
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_ ¡¨sþsocket[30] ,
            max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "SERVER: WELCOME TO SERVER \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY ¡¨sþ
    address.sin_port = htons( serverPort );

    //bind the socket to localhost port serverPort
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    print("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
      ¡¨sþ           FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            print("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            // printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , n ¡¨sþew_socket , inet_ntoa(address.sin_addr) , ntohs
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

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &re ¡¨sþadfds))
            {
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
          ¡¨sþ           print(buffer); print("\n");
                    if(strcmp(buffer, UPLOAD_COMMAND) == 0){
                        handleUploadToServer(sd);
                    }else if(strcmp(buffer, DOWNLOAD_COMMAND) == 0){
                        handleDownloadFromServer(sd);
                    }
                }
            }
        }
    }

    return 0;
}

int main(int argc, char** argv) {

    if(argc != 2){
        print("ERROR: Server needs 1 argument!\n");
        exit(-1);
    }
    int heartbeatPort = strtol(argv[1], NULL, 10);

    startHeartbeat(heartbeatPort);
    runServer(atoi(SERVER_PORT));

    while (1){

    }

    close(globalHeartbeatSocketFD);

    return 0;
}