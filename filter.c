//
//  filter.c
//  a3
//
//  Created by Alice on 2015-03-29.
//  Copyright (c) 2015 cs317. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int i;

int blacklist_check(char* hostinfo, char* filename){
    
    long lSize;
    char *buffer;
    char path[80];
    strcpy (path,"./");
    strcat(path,filename);
    FILE * fp = fopen( path, "r" );
    if( !fp ) {
        perror("Cannot not open the blacklist file. \n");
        return -1;
    }
    
    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );
    
    /* allocate memory for entire content */
    buffer = (char*)malloc(lSize+1);
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
    
    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);
    
    /* Split string into list of stings by white space and comma */
    
//    printf("%s\n", buffer);
    char* pch;
    pch = strtok (buffer," ,\n");
    
    char* match = (void*)0;
    
    while ((pch != (void*)0) && (match == (void*)0)){
        match = strstr(hostinfo, pch);
        printf ("%s\n",pch);
        pch = strtok (NULL," ,\n");
    }
    printf("We find a match in the blacklist: %s\n", match);
    
    
    if (match == (void*)0){
        i = 0;
    }else{
        i = 1;
    }
    fclose(fp);
    free(buffer);
    
    return i;
}


