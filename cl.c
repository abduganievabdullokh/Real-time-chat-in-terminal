#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sockfd;
struct termios oldt;

// Function to restore terminal settings on exit
void restore_terminal_settings() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    close(sockfd);
}

// Signal handler for SIGINT to restore terminal settings
void sigint_handler(int sig) {
    restore_terminal_settings();
    exit(0);
}

void *receiveMessages(void *socket) {
    int sockfd = *((int *)socket);
    char buffer[BUFFER_SIZE];
    int valread;

    while ((valread = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        buffer[valread] = '\0';
        printf("%s", buffer);
       // printf("> ");
        fflush(stdout);
    }

    return NULL;
}

int main() {
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to the server. Type your messages below:\n> ");

    pthread_create(&recv_thread, NULL, receiveMessages, (void *)&sockfd);

    // To capture each keystroke
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Set up signal handler to restore terminal settings on exit
    signal(SIGINT, sigint_handler);

    char ch;
    while (1) {
        ch = getchar();
        buffer[0] = ch;
        buffer[1] = '\0';
        send(sockfd, buffer, strlen(buffer), 0);
        printf("%c", ch);  // Echo the character back to the user
        fflush(stdout);
    }

    // Restore old terminal settings
    restore_terminal_settings();

    return 0;
}

