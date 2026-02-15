#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "QUIZDB_H.h"
#include "socket_io.h"

#define BACKLOG 10

int main(int argc, char *argv[])
{
    if (argc != 3) {
       fprintf(stderr, "Usage: %s <IPv4 address> <port number>.\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    
    char SERVERIP[20];
    strcpy(SERVERIP, argv[1]);

    struct sockaddr_in serverAddress;

    memset(&serverAddress, 0, sizeof(struct sockaddr_in));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(argv[2]));

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
       fprintf(stderr, "socket() error.\n");
       exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
       exit(EXIT_FAILURE);
    }

    int rc = bind(lfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr));
    if (rc == -1) {
       fprintf(stderr, "bind() error.\n");
       exit(EXIT_FAILURE);
    }

    if (listen(lfd, BACKLOG) == -1) {
       exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Listening on (%s, %s)\n", SERVERIP, argv[2]);

    for (;;) {
        fprintf(stdout, "<waiting for clients to connect>\n");
        fprintf(stdout, "<ctrl-C to terminate>\n");

        struct sockaddr_storage claddr;
        socklen_t addrlen = sizeof(struct sockaddr_storage);
        int cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
        if (cfd == -1) {
           continue;
        }

        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *) &claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
           fprintf(stdout, "Connection from (%s, %s)\n", host, service);
        } else {
           fprintf(stderr, "Connection from (?UNKNOWN?)");
        }
        
        char buf[BUFSIZE] = "\nWelcome to Unix Programming Quiz!\nThe quiz comprises of five questions posed to you one after the other.\nYou have only one attempt to answer a question.\nYour final score will be sent to you after conclusion of the quiz.\nTo start the quiz, press Y and <enter>.\nTo quit quiz, press q and <enter>.";

        writeToSocket(buf, cfd);
        readFromSocket(buf, cfd);

        if (strcmp(buf, "q") == 0) {
           fprintf(stderr, "User decided to quit.\n");
        } else {
            int correct = 0;
            int count = 0;
            int questionNumber = 0;
            srand(time(0));
            int questionsAsked[5] = {-1,-1,-1,-1,-1};

            while (count < 5) {
              int checker = 0;

              while (checker == 0) {
                checker = 1;
                int i = 0;
                questionNumber = (rand() % 43);
                
                while (i < 5) {
                  if (questionNumber == questionsAsked[i]) {
                    checker = 0;
                  }
                  i++;
                }
              }
              questionsAsked[count] = questionNumber;
              strcpy(buf, QuizQ[questionNumber]);
              printf("\nQuestion %d:\n%s\n", (count+1), buf);
              writeToSocket(buf, cfd);

              readFromSocket(buf, cfd);
              if (strcmp(buf, QuizA[questionNumber]) == 0) {
                 strcpy(buf, "Correct Answer");
                 correct++;
                 writeToSocket(buf, cfd);
              } else {
                   snprintf(buf, BUFSIZE, "Wrong Answer. Right answer is %s.", QuizA[questionNumber]);
                   writeToSocket(buf, cfd);
                }

              count++;
            }
            snprintf(buf, BUFSIZE, "Your quiz score is %d/5. Goodbye!", correct);
            writeToSocket(buf, cfd);
        }

        if (close(cfd) == -1) {
           fprintf(stderr, "close error.\n");
           exit(EXIT_FAILURE);
        }
    }

    if (close(lfd) == -1) {
       fprintf(stderr, "close error.\n");
       exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
