#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

char *str_slice(char *src, int begin, int len);
void executeCommand(char *command, int fd);

int main(int argc, char *argv[]) {
    bool isFirst = true;
    int port, sockFd, newFd, valRead, j;
    struct sockaddr_in myAddr;
    char opt, username[20], password[20], *client_username, *client_password, buffer[1024];
    buffer[0]='\0';

    if (argc < 7){
        printf("Usage: -p port -u username -p password\n");
        return -1;
    }


    while ((opt = getopt(argc, argv, "p:u:p:")) != -1){
        switch (opt){
            case 'p':
                if (isFirst){
                    port = atoi(optarg);
                    printf("port option passed as %d\n", port);
                    isFirst = false;
                } else{
                    strcpy(password, optarg);
                    printf("password passed as %s\n", password);
                }
                break;
            case 'u':
                strcpy(username, optarg);
                printf("username passed as %s\n", username);
                break;
            default:
                printf("usage is not true\n");
                exit(1);
        }
    }
    sockFd = socket(PF_INET, SOCK_STREAM, 0);
    memset(&myAddr, '0', sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(port);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    

    if (bind(sockFd,(struct sockaddr *) &myAddr, sizeof(struct sockaddr))){
        perror("bind failed");
        exit(1);
    }
    if (listen(sockFd, 3)){
        perror("listen failed");
        exit(1);
    }
    int myAddrLen = sizeof(myAddr);
    if ((newFd = accept(sockFd, (struct sockaddr *)&myAddr, (socklen_t *) &myAddrLen))<0){
        perror("accept failed");
        exit(1);
    }
    valRead = recv(newFd, buffer, 1024, 0);
    buffer[valRead] = '\0';
    printf("server received: %s %d\n",buffer, valRead);

    client_username = str_slice(buffer,2, atoi(str_slice(buffer,0,2)));
    j = strlen(client_username) + 2;
    client_password = str_slice(buffer, j + 2, atoi(str_slice(buffer, j, 2)));


    char msg[10];
    msg[0]='1';msg[1]='\0';
    if (strcmp(client_username, username) == 0 && strcmp(client_password, password) == 0){
        send(newFd , msg , strlen(msg) , 0 );
        printf("Authentication message sent\n");
        while (true){
            valRead = recv(newFd, buffer, 1024, 0);
            buffer[valRead] = '\0';
            if (strcmp(buffer, "q") == 0)
                return 0;
            printf("server received: %s\n", buffer);
            executeCommand(buffer, newFd);
        }

    }
    else{
        msg[0] = '0';
        send(newFd, msg, strlen(msg), 0);
    }

    close(newFd);
    close(sockFd);
    return 0;
}

void executeCommand(char *command, int fd){
    FILE *fp;
    char buff[1035];
    int sent;

    // Redirect stderr to stdout
    strcat(command, " 2>&1");
    
    // Open the command for reading.
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    // Read the output line by line, send through socket.
    while (fgets(buff, sizeof(buff), fp) != NULL) {
        if ((sent = send(fd, buff, strlen(buff), 0)) != strlen(buff)){
            perror("send command output");
        }
        //printf("aaa: %s", buff);
    }
    // Send predefined message which declares output has finished.
    strcpy(buff, "ENDOFCOMMAND");
    if ((sent = send(fd, buff, strlen(buff), 0)) != strlen(buff)){
        perror("send command output finished");
    }

    // close file pointer
    pclose(fp);
}

char *str_slice(char *src, int begin, int len){
    char *k = malloc(len + 1);

    memmove(k, src + begin, len);
    k[len]='\0';

    return k;
}
