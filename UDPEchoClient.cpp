//  UDPEchoClient.cpp
//  $ g++ -std=c++14 UDPEchoClient.cpp -o UDPEchoClient
//  ./UDPEchoClient [server-name] [Port] [Message] [File-name]
//  ./UDPEchoClient dockercompose-ubuntu-2004_server-1 8080 "Hello" simple-file.dat

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 8080
#define MAXLINE 1024
// #define MAXLINE 13442
#define HOST "127.0.0.1"
#define DEFAULT_MESSAGE "Test message from client"

void send_file_data(FILE *fp, int sockfd, struct sockaddr_in addr);

int main(int argc, char *argv[])
{
  int port = PORT;
  int sockfd;
  char *host;
  char host_ip[20];
  char buffer[MAXLINE];
  char *pmessage;
  int n;
  int s;
  FILE *fp;
  char *filename;
  struct sockaddr_in servaddr, clientaddr;
  int len = sizeof(clientaddr);
  // Read the arguments from command line
  if (argc >= 2)
  {
    // 1nd parameter is host
    host = argv[1];

    if (argc >= 3)
    {
      // 2rd parameter is port
      port = atoi(argv[2]);
    }
    if (argc >= 4)
    {
      // 3rd parameter
      pmessage = argv[3];
    }
    if (argc == 5)
    {
      // 4rd parameter
      filename = argv[4];
    }
  }

  // creating UDP socket. If any error occurred will print the error message.
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("Socket creation error");
    exit(EXIT_FAILURE);
  }

  // zero sockaddr_in structs
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&clientaddr, 0, sizeof(clientaddr));

  // filling server information.
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = INADDR_ANY;

#if 0
    // ip address only
    inet_aton(host, &servaddr.sin_addr);

#else
  // dns names accepted
  struct hostent *he;
  struct in_addr **AddrList;

  if ((he = gethostbyname(host)) == NULL)
  {
    printf("gethostbyname error ... using original\n");
    strcpy(host_ip, host);
  }
  else
  {
    AddrList = (struct in_addr **)he->h_addr_list;
    strcpy(host_ip, inet_ntoa(*AddrList[0]));
  }
  if (inet_pton(AF_INET, host_ip, &servaddr.sin_addr) <= 0)
  {
    printf("Invalid address/address not supported\n");
    exit(EXIT_FAILURE);
  }

#endif

  int client_input = 1;
  while (client_input != 3)
  {
    printf("What do you like to send? \n 1.File \n 2.Message \n 3.Exit\n");

    // Input from the client as number
    scanf("%d", &client_input);
    int num = client_input;

    //  Sending the client choice as a number
    //  1. Send a file
    //  2. Send a message
    //  3. Exit
    sendto(sockfd, &num, sizeof(num), 0, (const sockaddr *)&servaddr, sizeof(servaddr));

    switch (num)
    {
    case 1:
    {
      fp = fopen(filename, "r");
      if (fp == NULL)
      {
        perror("[ERROR] reading the file");
        exit(EXIT_FAILURE);
      }

      // Sending file name to the server
      n = sendto(sockfd, filename, strlen(filename), 0, (const sockaddr *)&servaddr, sizeof(servaddr));
      printf("filename: %s was sent\n", filename);

      // Sending file data to the server
      send_file_data(fp, sockfd, servaddr);
      printf("File transfer complete.\n");

      // Resiving a message form server
      n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (sockaddr *)&clientaddr, (socklen_t *)&len);
      buffer[n] = '\0';
      printf("[Message-From-Server]: File was received\n");
      break;
    }
    case 2:
    {
      //   Sending the message to the server
      n = sendto(sockfd, pmessage, strlen(pmessage), 0, (const sockaddr *)&servaddr, sizeof(servaddr));
      printf("%d bytes sent\n", n);
      printf("The message was sent.\n");
      //  Resiving a message form server
      n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (sockaddr *)&clientaddr, (socklen_t *)&len);
      buffer[n] = '\0';
      printf("[Message-From-Server]: Message was received\n");
      break;
    }
    case 3:
    {
      // Exit
      close(sockfd);
      break;
    }
    }
    return 0;
  }
  // **************************************************************************
}

void send_file_data(FILE *fp, int sockfd, struct sockaddr_in addr)
{
  char buffer[MAXLINE];
  int s;
  // Sending the data
  // We read the text file data within a while loop.
  while (fgets(buffer, MAXLINE, fp) != NULL)
  {
    // we send that data to the server using the sendto function.
    s = sendto(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&addr, sizeof(addr));
    // If any error occurred while sending the data
    if (s == -1)
    {
      perror("[ERROR] sending data to the server.");
      exit(EXIT_FAILURE);
    }
    bzero(buffer, MAXLINE);
  }
  // Sending the 'END'. The END is used to tell the server that the file transfer is complete.
  strcpy(buffer, "END");
  s = sendto(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&addr, sizeof(addr));
  // we close the file pointer fp and return.
  fclose(fp);
  return;
}