#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT_NUM 8888
#define IP_NUM "127.0.0.1"


void do_thing_client_TCP(int);
void do_thing_server_TCP(int);
void do_thing_client_UDP(int, struct sockaddr_in);
void do_thing_server_UDP(int);
int start_game();

// error handlers

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res == -1) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res == -1) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        printf("inet_pton failed: src does not contain a character"
            " string representing a valid network address in the specified"
            " address family\n");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
}

void Shutdown(int sockfd, int flag) {
    int res = shutdown(sockfd, flag);
    if (res == -1) {
        perror("shutdown failed");
        exit(EXIT_FAILURE);
    }
}

void Close(int sockfd) {
    int res = close(sockfd);
    if (res == -1) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }
}

// actual program

void server_TCP() {
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    Bind(server, (struct sockaddr *) &adr, sizeof(adr));

    Listen(server, 1);
    socklen_t adrlen = sizeof (adr);
    int fd = Accept(server, (struct sockaddr *) &adr, &adrlen);
    

    do_thing_server_TCP(fd);
    Shutdown(fd, SHUT_RDWR);
    Shutdown(server, SHUT_RDWR);
}

void server_UDP() {
    int server = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    Bind(server, (struct sockaddr *) &adr, sizeof(adr));

    do_thing_server_UDP(server);
    Close(server);
}

void client_TCP() {
    int client = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    Inet_pton(AF_INET, IP_NUM, &adr.sin_addr);

    Connect(client, (struct sockaddr *) &adr, sizeof adr);
    do_thing_client_TCP(client);
    Shutdown(client, SHUT_RDWR);
}

void client_UDP() {
    int client = Socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in adr = {0}; //server's adr
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM); //server's port
    Inet_pton(AF_INET, IP_NUM, &adr.sin_addr); //it doesn't matter, we have 0

    do_thing_client_UDP(client, adr);
    Close(client);
}

void do_thing_server_TCP(int server) {
    int secret_num = start_game();

    while (1) {
        int buf, hit = 0;
        read(server, &buf, sizeof(buf));
        printf("Number from client %d\nServer number %d\n", buf, secret_num);

        if (buf == secret_num) {
            hit = 1;
            printf("Client guessed your number\nServer shut down\n");
            write(server, &hit, sizeof(hit));
            return;
        } else {
            printf("Client missed\n\n");
            write(server, &hit, sizeof(hit));
        }
    }
}

void do_thing_server_UDP(int server) {
    int secret_num = start_game();
    struct sockaddr_in adr = {0};

    while (1) {
        int buf, hit = 0;
        int adr_len = sizeof(adr);
        recvfrom(server, &buf, sizeof(buf), 0, (struct sockaddr *) &adr, &adr_len);
        printf("Number from client %d\nYou number %d\n", buf, secret_num);

        if (buf == secret_num) {
            hit = 1;
            printf("Client guessed your number\nServer shut down\n");
            sendto(server, &hit, sizeof(hit), 0, (struct sockaddr *) &adr, adr_len);
            return;
        } else {
            printf("Client missed\n\n");
            sendto(server, &hit, sizeof(hit), 0, (struct sockaddr *) &adr, adr_len);
        }
    }
}

void do_thing_client_TCP(int client) {
    printf("Hi! you are client\nTry to guess server number\n\n");

    while (1) {
        int buf;
        scanf("%d", &buf);
        write(client, &buf, sizeof(buf));
        read(client, &buf, sizeof(buf));

        if (buf) {
            printf("You guessed!!!\nClient shut down\n");
            return;
        } else {
            printf("You missed\n\n");
        }
    }
}

void do_thing_client_UDP(int client, struct sockaddr_in adr) {
    printf("Hi! you are client\nTry to guess server number\n\n");

    while (1) {
        int buf;
        int adr_len = sizeof(adr);
        scanf("%d", &buf);

        sendto(client, &buf, sizeof(buf), 0, (struct sockaddr *) &adr, adr_len);
        recvfrom(client, &buf, sizeof(buf), 0, (struct sockaddr *) &adr, &adr_len);
        if (buf) {
            printf("You guessed!!!\nClient shut down\n");
            return;
        } else {
            printf("You missed\n\n");
        }
    }
}

int start_game() {
    printf("Hi! You are server\nTry to think of number from 0 to 10\n"
            "Client will try to guess it\nYou also can use rand"
            "\n1 - for rand\n2 - to input your own num\n\n");

    int buf;
    scanf("%d", &buf);
    if (buf == 1) {
        srand(time(NULL));
        return rand() % 10;
    } else if (buf == 2) {
        scanf("%d", &buf);
        printf("\nI have your number\n\n");
        return buf;
    }
    return 0;
}


int main() {
    int config;
    printf("Who are you\n1 - server_TCP\n2 - server_UDP\n3 - client_TCP\n4 - client_UDP\n");
    scanf("%d", &config);
    if (config == 1) {
        server_TCP();
    } else if (config == 2) {
        server_UDP();
    } else if (config == 3) {
        client_TCP();
    } else if (config == 4) {
        client_UDP();
    }

    return 0;
}
