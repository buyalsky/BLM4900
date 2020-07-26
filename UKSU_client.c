#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <stdbool.h>

bool authenticate(char *username, char *password, int fd);
char *encode(char *username, char *password );

int main(int argc, char *argv[]){
    printf("UKSU_client\n");
    int client_fd, port, valRead;
    bool isFirst = true;
    char opt, buffer[1024] = {0}, host_addr[20], username[20], password[20];
    struct sockaddr_in client_addr;

    if (argc < 9){
        printf("Usage: -h host_address -p port -u username -p password\n");
        return -1;
    }

    while ((opt = getopt(argc, argv, "h:p:u:p:")) != -1){
        switch (opt){
            case 'h':
                strcpy(host_addr, optarg);
                break;
            case 'p':
                if (isFirst){
                    port = atoi(optarg);
                    printf("port option passed as %d\n", port);
                    isFirst = false;
                }else{
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
                return -1;
        }
    }

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }
    struct hostent *he;
    if((he=gethostbyname(host_addr)) == NULL){
        perror("gethostbyname");
        return -1;
    }
    memset(&client_addr, '0', sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);
    client_addr.sin_addr = *((struct in_addr *)he->h_addr);

    if(connect(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0){
        printf("connection failed");
        return -1;
    }

    if(!authenticate(username, password, client_fd)){
        printf("Authentication failed\n");
        return -1;
    }else{
        while (true){
            printf("\ntype command (enter q to quit) :\n");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strlen(buffer) - 1] = '\0';

            if (strcmp(buffer, "q")==0){
                send(client_fd, buffer, strlen(buffer), 0);
                return 0;
            }
            if (send(client_fd, buffer, strlen(buffer), 0) != strlen(buffer))
                perror("send command");
            else{
                printf("\n");
                do{
                    valRead = recv(client_fd, buffer, 1024, 0);
                    buffer[valRead] = '\0';
                    if (strcmp(buffer, "ENDOFCOMMAND") != 0)
                        printf("%s",buffer);
                } while (strcmp(buffer, "ENDOFCOMMAND") != 0);
            }
        }
    }

    close(client_fd);
    return 0;
}

bool authenticate(char *username, char *password, int fd){
    int sent;
    char *buff = encode(username, password);
    printf("sending %s\n", buff);
    if ((sent = send(fd, buff, strlen(buff), 0)) != strlen(buff)){
        printf("failed to send buffer\n");
        return false;
    }else{
        int received = recv(fd, buff, 2, 0);
        buff[received]='\0';
        if (strcmp(buff, "1") == 0){
            printf("Authentication succeeded\n");
            return true;
        }else if (strcmp(buff, "0") == 0){
            return false;
        }else{
            perror("recv authentication result");
            return false;
        }
    }
}

char *encode(char *username, char *password ){
    int lenUser, lenPass, length;
    char *str, *str2;
    lenUser = strlen(username);
    lenPass = strlen(password);
    str2 = malloc(lenUser + lenPass + 4);
    str2[0] = '\0';

    length = snprintf( NULL, 0, "%d", lenUser );
    str = malloc( length + 1 );
    snprintf( str, length + 1, "%d", lenUser );
    if (length == 1){
        strcat(str2, "0");
        strcat(str2, str);
    } else
        strcat(str2, str);
    free(str);
    strcat(str2, username);
    length = snprintf( NULL, 0, "%d", lenPass );
    str = malloc( length + 1 );
    snprintf( str, length + 1, "%d", lenPass );
    if (length == 1){
        strcat(str2, "0");
        strcat(str2, str);
    } else
        strcat(str2, str);
    free(str);
    strcat(str2, password);

    return str2;
}
