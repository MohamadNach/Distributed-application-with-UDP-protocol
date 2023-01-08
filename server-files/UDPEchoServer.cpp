//  UDPEchoServer.cpp
//  $ g++ -std=c++14 UDPEchoServer.cpp -o UDPEchoServer
//  ./UDPEchoServer [Port]
//  ./UDPEchoServer 8080

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define MAXLINE 1024

FILE *fp;
char *filename;

void INThandler(int sig);

int main(int argc, char *argv[])
{

  int port = PORT;
  int sockfd;
  int len;
  int n;
  char buffer[MAXLINE];
  struct sockaddr_in servaddr, clientaddr;
  socklen_t addr_size;

  if (argc == 2)
  {
    port = atoi(argv[1]);
  }
  // Creating the socket on the server side
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    perror("Socket creation error");
    exit(EXIT_FAILURE);
  }
  printf("Socket created successfully : %d\n", sockfd);

  // zero sockaddr_in structs
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&clientaddr, 0, sizeof(clientaddr));

  // filling server info
  servaddr.sin_family = AF_INET; // ipv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  // Assign the address and port to the socket
  if (bind(sockfd, (const sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("Bind failure");
    exit(EXIT_FAILURE);
  }

  printf("Port %d was assigned to the socket successfully.\n", port);

  // setup ctrl+c ahndler
  signal(SIGINT, INThandler);

  printf("UDPEchoServer listening port %d\nPress ctrl+c to stop...\n", port);
  len = sizeof(clientaddr);

  char filename[50];
  // Creating a file name array and assign zero value.
  for (int i = 0; i <= 0; i++)
  {
    filename[i] = '\0';
  }

  int client_input = 1;
  while (true)
  {

    buffer[n] = '\0';
    char num;
    // Resiving the client choice form client server
    recvfrom(sockfd, &num, sizeof(num), 0, (struct sockaddr *)&clientaddr, (socklen_t *)&addr_size);
    client_input = num;
    switch (client_input)
    {
    case 1:
    {

      printf("[Server] case 1 was recived:\n");

      // Resiving file name to the server
      n = recvfrom(sockfd, filename, MAXLINE, 0, (struct sockaddr *)&clientaddr, (socklen_t *)&addr_size);
      buffer[n] = '\0';

      printf("Name of text file received :  %s\n", filename);

      // Resiving file data to the server
      n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&clientaddr, (socklen_t *)&addr_size);
      buffer[n] = '\0';
      printf("%s\n", buffer);
      fp = fopen(filename, "w");
      if (fp)
      {
        fwrite(buffer, MAXLINE, 1, fp);
        printf("File received successfully.\n");
        buffer[n] = '\0';
        //  Sending a message to the client server
        n = sendto(sockfd, buffer, sizeof(buffer), 0, (const sockaddr *)&clientaddr, sizeof(clientaddr));
        if (strcmp(buffer, "END") == 0)
        {
          break;
        }
      }
      else
      {
        printf("Cannot create the file.\n");
      }
      memset(filename, '\0', sizeof(filename));

      fclose(fp);
      break;
    };
    case 2:
    {
      printf("[Server] case 2 was recived:\n");

      //  Resiving a message form the client server
      n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (sockaddr *)&clientaddr, (socklen_t *)&addr_size);
      buffer[n] = '\0';
      if (n > 0)
      {
        printf("Client request received from %s using port %d: %s\n",
               inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, buffer);
      }
      n = sendto(sockfd, buffer, sizeof(buffer), 0, (const sockaddr *)&clientaddr, sizeof(clientaddr));
      break;
    };
    case 3:
    {
      close(sockfd);
      break;
    };
    }
  }

  return 0;
}

// this function to handle ending the server listening
void INThandler(int sig)
{
  char c;

  signal(sig, SIG_IGN);
  printf("\nDo you really want to quit? [y/n] ");
  c = getchar();
  if (c == 'y' || c == 'Y')
  {
    // UDP Connection is connectionless => no need to close it
    exit(0);
  }
  else
  {
    signal(SIGINT, INThandler);
  }
}
