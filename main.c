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

void server_TCP() {
    int server = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    bind(server, (struct sockaddr *) &adr, sizeof(adr));

    listen(server, 1);
    socklen_t adrlen = sizeof (adr);
    int fd = accept(server, (struct sockaddr *) &adr, &adrlen);

    do_thing_server_TCP(fd);
    shutdown(fd, SHUT_RDWR);
    shutdown(server, SHUT_RDWR);
}

void server_UDP() {
    int server = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    bind(server, (struct sockaddr *) &adr, sizeof(adr));

    do_thing_server_UDP(server);
    shutdown(server, SHUT_RDWR);
}

void client_TCP() {
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    inet_pton(AF_INET, IP_NUM, &adr.sin_addr);

    connect(client, (struct sockaddr *) &adr, sizeof adr);
    do_thing_client_TCP(client);
    shutdown(client, SHUT_RDWR);
}

void client_UDP() {
    int client = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM + 1);
    inet_pton(AF_INET, IP_NUM, &adr.sin_addr);
    bind(client, (struct sockaddr *) &adr, sizeof(adr));

    do_thing_client_UDP(client, adr);
    shutdown(client, SHUT_RDWR);
}

void do_thing_server_TCP(int server) {
    int secret_num = start_game();

    while (1) {
        int buf, hit = 0;
        read(server, &buf, sizeof(buf));
        printf("Number from clien %d\nServer number %d\n", buf, secret_num);

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
        printf("Number from clien %d\nYou number %d\n", buf, secret_num);

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

void do_thing_client_UDP(int client, struct sockaddr_in adr) {
    printf("Hi! you are client\nTry to guess server number\n\n");
    adr.sin_port = htons(PORT_NUM);

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