#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "decode_client_request.h"
#include "host_connect.h"

#define BUFLEN 255

// Usage:  ./tcp_echo_server [port]
int main(int argc, char **argv) {
    pid_t pid;
    int n, i, bytes_to_read; // n for read; i for write
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;
    char *bp, buf[BUFLEN];
    
    if(argc != 3)
    {
        printf("Using:\n\t%s <port> <black-list>\n", argv[0]);
        return -1;
    }
    
    port = atoi(argv[1]);
    printf("HTTP Proxy Server now listening on port %i ... \n", atoi(argv[1]));
    
    //Add filter list to the url_blacklist array.
    char *filename = argv[2];
    
    /* Create a stream socket. */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        fprintf(stderr, "Can't open the socket. \n" );
        exit(1);
    }
    
    /* Initialize the socket structure */
    
    memset((char*)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    
    
    /* Bind an address to the socket */
    
    if (bind(sd, (struct sockaddr*)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket. \n" );
        exit(1);
    }
    
    /* Receive from the client. */
    if (listen(sd,5) == -1){
        fprintf(stderr, "Can't listen client. \n" );
    }else if (listen(sd,1) == 0){
        printf("successful listen to client. \n");
    }
    
    
    client_len = sizeof(client);
    
accepting:
    new_sd = accept(sd,(struct sockaddr *)&client, &client_len);
    if (new_sd < 0) {
        fprintf(stderr, "Can't accept client. \n" );
        exit(1);
    }else{
        printf("See we have one client connected. \n");
    }
    
    /* Start communicating with client. */
    /* Create child process */
    pid = fork();
    if (pid < 0)
    {
        perror("ERROR on fork");
        exit(1);
    }
    
    if (pid == 0){
        memset(buf, 0,sizeof(buf));
        bytes_to_read = BUFLEN;
        bp = buf;
        
        while ((n = read(new_sd, buf, bytes_to_read)) > 0){
            
            
            printf("Received: %s", buf);
            
            // check valid for client msg, if not valid, send 405
            if (check_valid(buf)){
                printf("Valid client message, and now send it to server. \n");
                //decode request
                struct client_message* pr_clientmsg =  decode_request(buf);
                char* host = pr_clientmsg->host;
                char* path = pr_clientmsg->path;
                char* http = pr_clientmsg->http_version;
                int portnum = pr_clientmsg->port;
                
                struct stat  file_stat;
                // create a cache folder if there is no cache folder in the current directory
                if (stat("./cache/", &file_stat) == -1) {
                    mkdir("./cache/", 0700);
                }
                char cachename[255]; // TODO: we will need to check length here
                strcpy(cachename, host);
                strcat(cachename, path);
                printf("%s\n", cachename);
                
                // convert the url to an ASCII hex encoding of the file name
                
                
                char rfc3986[255] = {0};
                int i = 0;
                for (; i < 255; i++) {
                    rfc3986[i] = isalnum(i) ? i : 0;
                }
                
                char *tb = rfc3986;
                char cachename_encoded[255];
                char* enc = cachename_encoded;
                char* s = cachename;
                for (; *s; s++) {
                    if (tb[*s]) sprintf(enc, "%c", tb[*s]);
                    else        sprintf(enc, "%%%02X", *s);
                    while (*++enc);
                }
                
                printf("%s\n", cachename_encoded);
                
                char cachepath [255];
                sprintf(cachepath, "./cache/%s", cachename_encoded);
                printf("The path of the cache file is %s \n", cachepath);
                
                struct host_response* pr_response;
                // check if it is in the black list
                if (blacklist_check(host, filename) == 0){
                    
                    //check if there is one existing in the cache
                    if (access(cachepath, F_OK) == 0){
                        //file exists
                        printf("Requested file existed in the cache folder. \n");
                        FILE *cf = fopen(cachepath, "r");
                        do
                        {
                            memset(buf, 0,sizeof(buf));
                            if(fgets(buf, 4096, cf) != NULL)
                            {
                                // send it to the client
                                send(new_sd, buf, n, 0);
                            }
                        } while(fgets(buf, 4096, cf) != NULL);
                        fclose(cf);
                        
                    }else{
                        //file not exist
                        printf("Requested file does not exist in the cache folder, we will get it from host. \n");
                        pr_response = connect_host(host, path, http, portnum);
                        /*
                         Check the host reply code:
                         If it is 200, send the host response to client.
                         If it is not, send the host reply code to client to indicate that host refuse to connect or refuse to send file
                         */
                        if (pr_response->reply_code == 200){
                            printf("successful connect to host and get the response. \n");
                            
                        int cfd = -1;
                            
                        //write it to the local cache file before sending it to client
                            i = write(new_sd, pr_response->reply_code, sizeof(int));
                            i = write(new_sd, pr_response->content, pr_response->cont_len);
                            // free the content
                            free(pr_response->content);
                            
                        printf("In proxy, the reply code is: %d\n",pr_response->reply_code);// print server reply code

                        if (i > 0){
                            printf("Successful sent data to client. Now close the connection to client. \n");
                            disconnect_host();
                            goto close;
                        }else{
                            fprintf(stderr, "Unknow error occured in the transfering. \n" );
                            exit(1);
                        }
                    }else{
                        char invalid_server_response [] = "Fail to connect to the host.";
                        printf("%s\n", invalid_server_response);
                        i = write(new_sd, invalid_server_response, strlen(invalid_server_response));
                        goto close;
                    }
                    
                }
                printf("In proxy, the reply code is: %d\n",pr_response->reply_code);// print server reply code
            }else {
                char error403 [] = "Error: 403 \nThe requested host is banned. \n";
                i = write(new_sd, error403, strlen(error403));
                goto close;
            }
            
        }else{
            char error405 [] = "Error: 405 \nInvalid Client Request.\n";
            i = write(new_sd, error405, strlen(error405));
        }
        
        memset(buf, 0,sizeof(buf));
    }
    
    if (n < 0){
        fprintf(stderr, "Can't read from socket. \n" );
        exit(1);
    }
    
    /* Clean up. */
    close(new_sd);
    close(sd);
    
    return 0;
}else{
close:
    close(new_sd);
    
    goto accepting;
}

    close(sd);
    return 0;

}