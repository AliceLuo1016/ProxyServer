#ifndef __DECODE_CLIENT_REQUEST_H__
#define __DECODE_CLIENT_REQUEST_H__

struct client_message{
	char* host;
	char* path;
	int port;
	char* http_version;
};

void* decode_request(char* msg);
bool check_valid(char* msg);

#endif