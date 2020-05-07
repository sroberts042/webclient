#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>


#define DEFAULT_HTTP_PORT 80

typedef struct url_s
{
  unsigned short usPort;   // in host byte order
  char          *szServer; // allocated by parse_url(), must be freed by the caller
  char          *szFile;   // allocated by parse_url(), must be freed by the caller
} url_t;

// Globals
char *g_szMessage;
char *g_szServer;
unsigned short g_usPort;


// Function Prototypes
void parse_args(int argc, char **argv);

url_t parse_url(const char *szURL)
{
  url_t url;
  memset(&url, 0, sizeof(url));

    unsigned int urllen = strlen(szURL) + 1;
    url.szServer = (char*) malloc(urllen * sizeof(char));
    assert(NULL != url.szServer);
    url.szFile = (char*) malloc(urllen * sizeof(char));
    assert(NULL != url.szFile);

  char server[urllen];
  int result = sscanf(szURL, "http://%[^/]/%s", server, url.szFile);
  if (EOF == result)
  {
    fprintf(stderr, "Failed to parse URL: %s\n", strerror(errno));
    exit(1);
  }
  else if (1 == result)
  {
    url.szFile[0] = '\0';
  }
  else if (result < 1)
  {
    fprintf(stderr, "Error: %s is not a valid HTTP request\n", szURL);
    exit(1);
  }

  result = sscanf(server, "%[^:]:%hu", url.szServer, &url.usPort);
  if (EOF == result)
  {
    fprintf(stderr, "Failed to parse URL: %s\n", strerror(errno));
    exit(1);
  }
  else if (1 == result)
  {
    url.usPort = DEFAULT_HTTP_PORT;
  }
  else if (result < 1)
  {
    fprintf(stderr, "Error: %s is not a valid HTTP request\n", szURL);
    exit(1);
  }

    assert(NULL != url.szServer);
    assert(NULL != url.szFile);
    assert(url.usPort > 0);
  return url;
}

int main(int argc, char **argv)
{
  parse_args(argc, argv);
  fprintf(stderr, "Sending message \"%s\" to %s:%hu\n",
         g_szMessage, g_szServer, g_usPort);

  struct hostent* temp  = (struct hostent*)malloc(sizeof(struct hostent));
  temp = gethostbyname(g_szServer);
  
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_aton(g_szServer, &addr.sin_addr);
  addr.sin_port = htons(g_usPort);

  memcpy(&addr.sin_addr, temp->h_addr_list[0], temp->h_length);
  if(!temp)
  {
    fprintf(stderr, "ERROR: could not get host\n");
    exit(-1);
  }

  //create socket and check for error
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) 
  {
    fprintf(stderr, "ERROR: client socket creation failed\n");
    exit(-1);
  }
  else
  {
    fprintf(stderr, "SUCCESS: client socket created\n");
  }

  //create connection and check for error
  int connection = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  if(connection < 0)
  {
    fprintf(stderr, "ERROR: connection failed\n");
    fprintf(stderr,"%d\n", errno);
    close(sock);
    exit(-1);
  }
  else 
  {
    fprintf(stderr, "SUCCESS: connection established\n");
  }

  //send message and check for error
  int message, i;
  for(i = 0; i < strlen(g_szMessage); i++)
  {
    message = send(sock, g_szMessage + i, sizeof(*g_szMessage), 0); 
  }
  if(message < 0)
  {
    fprintf(stderr, "ERROR: message not sent\n");
    close(sock);
    exit(-1);
  }
  else 
  {
    fprintf(stderr, "SUCCESS: message sent\n");
    char* buff = (char*)malloc(128000);
    int received_message, message_body,length = 0;

    //get message
    received_message = recv(sock, buff, 1, 0);

    while(!(strstr(buff, "\r\n\r\n")))
    {
        received_message += recv(sock, buff + received_message, 1, 0);
    }
    char* cont_len_str = (char*)malloc(1024);
    int place = 0;
    int cont_len_int = 0;
    char* sub = strstr(buff, "Content-Length:");
    if(sub > 0)
    {
          char* ret = "\r\n";
          while((buff[(sub - buff) + 16 + place]) != *ret)
          {
            cont_len_str[place] = buff[(sub - buff) + 16 + place];
            place++;
          }
          cont_len_int = atoi(cont_len_str);
    }
    else if(cont_len_int < 1)
    {
        fprintf(stderr, "ERROR: No Content-Length field in header\n");
        close(sock);
        exit(-1);
    }
    length = 1;
    buff = (char*)realloc(buff, cont_len_int + received_message);
    message_body = recv(sock, buff + received_message, cont_len_int, 0);
    while(message_body < cont_len_int)
    {
        length = recv(sock, buff+message_body, cont_len_int, 0);
        message_body += length;
    }


    //print header to stderr and body to stdout
    fwrite(buff, 1, received_message, stderr);
   
    fwrite(buff + received_message, 1, message_body, stdout);

    //check for error in received_message
    if(received_message < 0)
    {
        fprintf(stderr, "ERROR: no message received\n");
        exit(-1);
    }
  //  free(buff);
  }
  //close connection
  close(sock);
  //free(g_szMessage);
  return 0;
}


void parse_args(int argc, char **argv)
{
  errno = 0;

  if (argc < 2)
  {
    fprintf(stderr,
            "Usage: http_client URL\n");
    exit(1);
  }

  // Parse the URL
  url_t url = parse_url(argv[1]);
  fprintf(stderr, "Server: %s:%hu\nFile: /%s\n",
          url.szServer, url.usPort, url.szFile);

  g_usPort = url.usPort;
  g_szMessage = (char*)malloc(1024);
  g_szServer = url.szServer;
  sprintf(g_szMessage, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url.szFile, url.szServer);
}
