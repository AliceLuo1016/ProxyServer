#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "decode_client_request.h"

#define BUF_SIZE 128
bool check_valid(char* msg);
char* str_to_upper(char* str);
char* remove_prefix(char* url, char* raw_url);
const char delim_char = ' ';
int argc = 3;

//char request[BUF_SIZE];

//char* msg = "get http://www.fedex.southwest.com/delivery/box.html:8200 HTTP/1.1";

void* decode_request(char* msg) {
    
	//printf("%s\n", msg);
    
	char a[BUF_SIZE], b[BUF_SIZE], c[BUF_SIZE];
	sscanf(msg, "%s %s %s", a, b, c);

	//printf("%s %s\n", a, b);
    
	//fprintf(stdout, "1st arg: %s\n", a); // should be "get"
	//fprintf(stdout, "2st arg: %s\n", b); // should be an URL
	//printf("Number of args: %d\n", argc - 1);
    
	/*
     if (argc < 3) {
     // Tell the user there are insufficient arguments
     const char *err_msg = "insufficient args";
     printf("Error: number of args = %d; %s\n", argc - 1, err_msg);
     return -1;
     }
     */
	
	// if (!check_valid(a, b, c)) {
	// 	const char *err_msg = "405 Invalid Command";
	// 	printf("%s\n", err_msg);
	// 	return err_msg;
	// }
	
	// Decode the URL
	char* url = (char*)malloc(BUF_SIZE);
	remove_prefix(b, url);

	//strcpy(url, b);

	size_t i = 0, len = strlen(url);
	int divider = 0;
	while ((i < len) && (divider == 0)) {
		if (url[i] == '/') {
			divider = i;
			//printf("/: %c %d %d\n", url[i], i, divider);
		}
		i++;
	}
    
	// if (divider == 0) {
	// 	const char *err_msg = "invalid URL format";
	// 	printf("Error: URL: %s; %s\n", url, err_msg);
	// 	return -1;
	// }
    
	char* real_host = (char*)malloc(BUF_SIZE);
	char* file = (char*)malloc(BUF_SIZE);
    
	strncpy(real_host, url, divider);
	real_host[divider] = '\0';
	strncpy(file, url + divider, len);
    
	//printf("host: %s\n", real_host);
	//printf("file: %s\n", file);
    
	
	//char *request = (char*)malloc(BUF_SIZE);
	//snprintf(request, strlen(cmd) + 2, "%s ", cmd);
	//strcat(request, file);
	//const char *http_version = "HTTP/1.1\n";
	//strcat(request, http_version);
    
	//char host_line[BUF_SIZE];
	//const char *host_cmd = "Host: ";
	//snprintf(host_line, strlen(host_cmd) + 2, "%s ", host_cmd);
	//strcat(host_line, real_host);
	//strcat(request, host_line);

	int j = 0, flag = 0;
	for (j = 0; j < strlen(file); j++) {
		if (file[j] == ':') {
			flag = 1;
			break;
		}
	}

	//printf("%d\n", j);

	char* port_str = (char*)malloc(BUF_SIZE);
	char* clean_url = (char*)malloc(BUF_SIZE);

	int port_num = 0;

	if (flag) {
		strncpy(port_str, &file[j + 1], strlen(file));
		port_num = atoi(port_str);
		strncpy(clean_url, file, j);
		clean_url[j] = '\0';
	}
	else {
		port_num = 80;
		strncpy(clean_url, file, strlen(file));
	}

	//printf("port number is = %d\n", port_num);
	
	struct client_message cm;
	cm.host = (char*)malloc(BUF_SIZE);
	cm.path = (char*)malloc(BUF_SIZE);
	cm.http_version = (char*)malloc(BUF_SIZE);
    
	strcpy(cm.host, real_host);
	strcpy(cm.path, clean_url);
	strcpy(cm.http_version, c);
	cm.port = port_num;
	
	struct client_message *pr_message = &cm;
	

//	printf("%s\n", cm.host);
//	printf("%s\n", cm.path);
//	printf("%s\n", cm.http_version);
//	printf("%d\n", cm.port);

    
//	free(cm.http_version);
//	free(cm.path);
//	free(cm.host);
	free(clean_url);
	free(port_str);
	free(file);
	free(real_host);
	free(url);
    
	//return request;
	return pr_message;
    
}

char* str_to_upper(char* str) {
	size_t i, len = strlen(str);
	for (i = 0; i < len; i++) {
		str[i] = toupper((unsigned char)str[i]);
	}
	return str;
}

bool check_valid(char* msg) {
	char cmd[BUF_SIZE], host[BUF_SIZE], http[BUF_SIZE];
	sscanf(msg, "%s %s %s", cmd, host, http);
    
	bool valid = false;
	char* get = (char*)malloc(strlen(cmd));
	strcpy(get, cmd);
	str_to_upper(get);
	if (((strncmp(get, "GET", 3) == 0)) && 
		((strncmp(http, "HTTP/1.1", 8) == 0) ||
		(strncmp(http, "HTTP/1.0", 8) == 0))) {
		valid = true;
	}

	char* url = (char*)malloc(BUF_SIZE);
	remove_prefix(host, url);
	size_t i = 0, len = strlen(url);
	int divider = 0;
	while ((i < len) && (divider == 0)) {
		if (url[i] == '/') {
			divider = i;
		}
		i++;
	}
    
	if (divider == 0) {
		valid = false;
	}


	free(url);
	free(get);

	return valid;
}

char* remove_prefix(char* url, char* raw_url) {
	if (strncmp(url, "http://", 7) == 0) {
		strncpy(raw_url, &url[7], strlen(url));
		raw_url[strlen(url)] = '\0';
	}
	else{
		strcpy(raw_url, url);
	}
	return raw_url;
}
