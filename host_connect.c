//
//  host_connect.c
//  a3
//
//  Created by Alice on 2015-03-29.
//  Copyright (c) 2015 cs317. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include "host_connect.h"
#include <stdlib.h>
#include <ctype.h>

int host_sd;

void* connect_host(char* host, char* path, char* version, int port)
{
    int host_port, n;
    struct sockaddr_in host_addr;
    struct hostent *server;
    
    char buffer[255];
    
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
    
    host_port = port;
    
    /* Create a socket point */
    host_sd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (host_sd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    
    //This part server will be decoded message from client
    server = gethostbyname(host);
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &host_addr, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&host_addr.sin_addr.s_addr, server->h_length);
    host_addr.sin_port = htons(host_port);
    
    /* Now connect to the server */
    if (connect(host_sd,(struct sockaddr*)&host_addr,sizeof(struct sockaddr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }
    
    /* Now ask for a message from the user, this message
     * will be read by server
     */
    bzero(buffer,255);

    /* Send message to the server */
    sprintf(buffer,"GET %s %s\r\nHost: %s\r\n\r\n",path, version, host);
    n = write(host_sd,buffer,strlen(buffer));
    printf("\n%s\n",buffer);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
    
    /* Now read server response */
    
//get_response:
    
    bzero(buffer,255);
    n = read(host_sd,buffer,255);
    
    if (n < 0)
    {
        perror("ERROR reading from socket");
        exit(1);
//        goto end;
    }
    printf("%s\n",buffer);
    
    // Get the reply code and content length
    char m1[255], reply_code[10], m2[255], len[10];
    sscanf(buffer,"%s %s %s",m1,reply_code,m2);
    char *con_len = "Content-Length";
    char *p_len = strstr(buffer, con_len);
    
    sscanf(p_len, "%s %s %s",m1,len,m2);
    
    int num = atoi(reply_code);
    long int content_length = atol(len);

    printf("Reply code: %d\n", num);
    printf("Content length: %ld\n", content_length);

    // struct one host response to contain all the information
    struct host_response hr;
    hr.reply_code = num;
    hr.cont_len = content_length;
    
    bzero(buffer,255);
    hr.content = (char*)malloc(content_length+1);
    

    FILE *cachefile = fopen(cachepath, "w");
    
    do {
        bzero(buffer,255);
        n = read(host_sd,buffer,255);
        printf("n is %d\n", n);
        strncat(hr.content, buffer, 255);
        fputs(buffer, cachefile);
        printf("%s\n",buffer);
    }while (n>0);

    printf("Reply code: %d\n", hr.reply_code);
    // printf("Content: %s\n", hr.content);
    
    struct host_response *pr_response = &hr;
//    goto get_response;
//    
//end:
    fclose(cachefile);
    return pr_response;
}

void disconnect_host(){
    close(host_sd);
    printf("The host is disconnected. \n");
}