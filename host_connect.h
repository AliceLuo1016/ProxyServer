//
//  host_connect.h
//  a3
//
//  Created by Alice on 2015-03-29.
//  Copyright (c) 2015 cs317. All rights reserved.
//

#ifndef __HOST_CONNECT_H__
#define __HOST_CONNECT_H__

struct host_response{
    int reply_code;
    char* content;
    long int cont_len;
};

void* connect_host(char* host, char* path, char* version, int port);
void disconnect_host();

#endif