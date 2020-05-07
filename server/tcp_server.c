#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

// Globals
unsigned short g_usPort;


// Function Prototypes
void parse_args(int argc, char **argv);


// Function Implementations

int main(int argc, char **argv)
{
    parse_args(argc, argv);
    printf("Starting TCP server on port: %hu\n", g_usPort);

    //set up connection and check for error
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock < 0)
    {
        printf("ERROR: server socket creation failed\n");
        exit(-1);
    }
    else
    {
        printf("SUCCESS: server socket created\n");
    }

    int client_sock;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(g_usPort);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind the socket and check for error
    int bound_socket = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bound_socket < 0)
    {
        printf("ERROR: server socket binding failed\n");
        close(client_sock);
        exit(-1);
    }
    else
    {
        printf("SUCCESS: server socket binding complete\n");
    }

    //listen for connection request and check for error
    int listening = listen(server_sock, 10);
    if(listening < 0)
    {
        printf("ERROR: no socket found\n");
        close(client_sock);
        exit(-1);
    }
    else
    {
        printf("SUCCESS: socket found\n");
    }

    //reveive message
    char* buff;
    int received_message;

    while(1)
    {
        buff = malloc(1024);
        received_message = 0;
        client_sock = accept(server_sock, (struct sockaddr*)NULL, NULL);

        //get message
        received_message = recv(client_sock, buff, 1024, 0);
        while(!strstr(buff, "\r\n\r\n"))
        {
            received_message += recv(client_sock, buff + received_message, 1024, 0);
        }
        //check for error in received_message
        /*  if(received_message < 0)
            {
            printf("ERROR: no message received\n");
            exit(-1);
            }
            else
            {
            printf("SUCCESS: message received\n");
            }*/
        //PARSE GET REQUEST TO MAKE SURE IT IS GET
        char* return_message = malloc(2048);
        //IF ERROR, ERROR CODE 400
        //IF NOT, ERROR CODE 501
        char* sub = malloc(1024);
        if(!(sub = strstr(buff, "GET"))) sprintf(return_message, "HTTP/1.1 501 NOT IMPLEMENTED\r\n\r\n");
        else{
            char* file_name = malloc(1024);
            memset(file_name, 0, 1024);
            
            int place = 0;
            //Get name of file then open it
            char* ret = "\r\n";
            while((buff[buff - sub + place + 5]) != *ret)
            {
                file_name[place] = buff[buff - sub + place + 5];
                place++; 
            }
            int k = 0;
            int file_name_length = strlen(file_name);
            for(k = 0; k < 10; k++)
            {
                file_name[file_name_length - k] = 0;
            }
            //IF YES, CHECK FOR FILE IN DIRECTORY
            //IF NO FILE RETURN 404
            struct stat st;
            memset(&st, 0, sizeof(st));
            char* file_loc = malloc(1024);
            sprintf(file_loc, "%s", file_name);
            printf("%s\n", file_loc);
            //find extension
            char* charDot = strstr(file_name, ".");
            
            /*int ext_loc = strstr(file_name, ".");
            ext_loc = (file_name - ext_loc);
            ext_loc = -ext_loc;
            char* ext;
            if(ext_loc > 0 && ext_loc <= strlen(file_name))
            {
                ext = malloc(20);
                int m;
                place = 0;
                for(m = ext_loc; m < strlen(file_name); m++)
                {
                    ext[place] = file_name[ext_loc + place];
                    place++;
                }
            }*/
            
            //find file type
            char* file_type;
            if(!strcmp(charDot, ".txt"))
            {
                file_type = "text/plain";
            }
            else if(!strcmp(charDot, ".html") || !strcmp(charDot, ".htm"))
            {
                file_type = "text/html";
            }
            else if(!strcmp(charDot, ".jpg"))
            {
                file_type = "image/jpg";
            }
            else if(!strcmp(charDot, ".jpeg"))
            {
                file_type = "image/jpeg";
            }
            else
            {
                file_type = "text/html";
            }      
            char mtime[80];
            time_t t = st.st_mtime;
            struct tm lt;
            localtime_r(&t, &lt);
            strftime(mtime, sizeof(mtime), "%a, %d, %b, %Y, %T", &lt);            

            char mtimen[80];
            time_t tn;
            struct tm* ltn;
            time(&tn);
            ltn = localtime(&tn);
            strftime(mtimen, sizeof(mtimen), "%a, %d, %b, %Y, %T", ltn);            
            int result = stat(file_loc, &st);

            sprintf(return_message, "HTTP/1.1 200 OK\r\nConnection: close\r\nDate: %s\r\nLast-Modified: %s\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", mtimen, mtime, (int)st.st_size, file_type);

            int total_size_of_message = strlen(return_message) + (st.st_size);
            return_message = realloc(return_message, strlen(return_message) + (st.st_size));

            char send_404 = 0;
            if(result < 0)
            {
                send_404 = 1;
                total_size_of_message = 26;
                sprintf(return_message,"HTTP/1.1 404 NOT FOUND\r\n\r\n");
            }
            //printf("%s\n", ctime(st.st_mtime));
            FILE* f = fopen(file_name, "rb");
            if(!f || result < 0)
            {
                send_404 = 1;
                total_size_of_message = 26;
                sprintf(return_message, "HTTP/1.1 404 NOT FOUND\r\n\r\n");
            }

            if(!send_404)
            {
                char* file_contents = malloc(st.st_size);

                //IF YES, RETRIEVE FILE
                int numBytes = 0;
                while(numBytes < st.st_size)
                {
                    numBytes += fread(file_contents, 1, st.st_size, f);
                }

                //PACKAGE FILE WITH METADATA
                memcpy((void*)(return_message + strlen(return_message)), (void*)file_contents, st.st_size);
                fclose(f);
            }

            //send message and check for error
            int message = 0;
            while(message < total_size_of_message)
            {
                if(message < 0)
                {
                    fprintf(stderr, "ERROR: message not sent\n");
                    close(client_sock);
                    exit(-1);
                }
                message += send(client_sock, return_message + message, total_size_of_message, 0);
            }
            fprintf(stderr, "SUCCESS: Message sent\n");
            
            /*free(file_loc);
            free(file_contents);
            free(buff);*/
        }
    }
    close(server_sock);
    close(client_sock);

    //close connection
    return 0;
}


void parse_args(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr,
                "tcp_server: missing port operand\nUsage: tcp_server PORT\n");
        exit(1);
    }

    errno = 0;
    char *endptr = NULL;
    unsigned long ulPort = strtoul(argv[1], &endptr, 10);

    if (0 == errno)
    {
        // If no other error, check for invalid input and range
        if ('\0' != endptr[0])
            errno = EINVAL;
        else if (ulPort > USHRT_MAX)
            errno = ERANGE;
    }
    if (0 != errno)
    {
        // Report any errors and abort
        fprintf(stderr, "Failed to parse port number \"%s\": %s\n",
                argv[1], strerror(errno));
        abort();
    }
    g_usPort = ulPort;
}
