//
// Created by Theodore Ahlfeld on 11/19/15.
//

//#include "bfclient.h"
#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <pthread.h>
#include <sys/errno.h>
#include "bf_node.h"
#include "routing_table.h"
#include "tuip.h"

#ifdef __APPLE__
    #define IPV4INTERFACE "en0"
#else
    #define IPV4INTERFACE "eno16777736"
#endif

#define FOUND 0

void die_with_err(const char *s)
{
    perror(s);
    exit(1);
}

addrinfo *create_udp_addr(const char *hostname, const char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG;
    struct addrinfo *addr = nullptr;
    if (getaddrinfo(hostname, port, &hints, &addr)) {
        perror("getaddrinfo() failed");
        return nullptr;
    }
    return addr;
}

int create_udp_socket(addrinfo *addr) {
    int sock = socket(addr->ai_family, (addr->ai_socktype),
                      (addr->ai_protocol));
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < -1) {
        die_with_err("socket() failed");
    }
    return sock;
}

void bind_udp(int sock, struct addrinfo *addr) {
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) == -1) {
        die_with_err("bind() failed");
    }
}

int open_udp_listening_socket(const char *port) {
    struct addrinfo *addr = create_udp_addr(nullptr, port);
    int sock = create_udp_socket(addr);
    bind_udp(sock, addr);
    freeaddrinfo(addr);
    return sock;
}

bool timer_send(time_t *interval)
{
    static time_t last_broadcast;
    time_t now;
    if(time(&now) >= (last_broadcast + *interval)) {
        last_broadcast = now;
        return true;
    }
    return false;
}

string tuip_id(const char *port)
{
    int sock;
    struct ifreq ifr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, IPV4INTERFACE, IFNAMSIZ-1);
    ioctl(sock, SIOCGIFADDR, &ifr);
    close(sock);
    char *ip = inet_ntoa(((sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    return string(ip)+string(":")+string(port);
}

void update_adj_vec(Tuip *tuip, RoutingTable *rt)
{
    std::vector<Tuip> *tuip_vec = tuip->get_rt();
    long long w = tuip->get_weight();
    Node *neighbor = rt->get_node(tuip->get_name());
    if (neighbor == nullptr) {
        return;
    }
    for(auto it = tuip_vec->begin(); it != tuip_vec->end(); ++it) {
        long long totalw = w + it->get_weight();
        if(totalw < rt->find_weight(it->get_name())) {
            Node *dst = rt->get_node(it->get_name());
            dst->update_route(neighbor);
            dst->set_weight(totalw);
        }
    }
}

void *dbfr_loop(void *kludged_data) {
    void **sock_rt_id_int = (void **)kludged_data;
    int lstn_sock = *(int *) sock_rt_id_int[0];
    sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    RoutingTable *rt = (RoutingTable *) sock_rt_id_int[1];
    string *id = (string *)sock_rt_id_int[2];
    time_t *interval = (time_t *) sock_rt_id_int[3];
    uint8_t buffer[4096];
    while (rt != nullptr) {
        if (timer_send(interval)) {
            rt->broadcast_all(id);
        }
        ssize_t len;
        while((len = recvfrom(lstn_sock, buffer, sizeof(buffer)-1,
                              0, (sockaddr *)&src_addr, &src_len)) > 0) {
            printf("recv: %s\n", buffer);
            buffer[len] = '\0';
            while(buffer[0] != '\0') {
                Tuip tuip = Tuip(buffer);
                tuip.set_weight(rt->find_weight(tuip.get_name()));
                update_adj_vec(&tuip, rt);
            }
        }
        if(len < 0 && errno != EAGAIN){
            printf("len = %ld\n", len);
            perror("recvfrom()");
            pthread_exit(nullptr);
        }
    }
    pthread_exit(nullptr);
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        die_with_err("Invalid amount arguments");
    }
    const char *local_port = argv[1];
    time_t interval = (time_t)atoi(argv[2]);
    RoutingTable *rt = new RoutingTable();
    if(argc%3) {
        die_with_err("Invalid triplet amount");
    }
    string id = tuip_id(local_port);
    int lstn_sock = open_udp_listening_socket(local_port);
    /*for(;;) {
        char buf[500];
        struct sockaddr_storage src_addr;
        socklen_t src_len = sizeof(src_addr);
        if(recvfrom(lstn_sock, buf, 500, 0,
                    (struct sockaddr *) &src_addr, &src_len)>0) {
            printf("%s\n", buf);
        }
    }*/
    for(int trip_idx = 3; trip_idx < argc; trip_idx += 3) {
        char *ip = argv[trip_idx];
        char *port = argv[trip_idx+1];
        long long weight = (long long)atol(argv[trip_idx+2]);
        addrinfo *addr = create_udp_addr(ip, port);
        int sock = create_udp_socket(addr);
        sendto(sock, "HELLO WORLD\n", 12, 0, addr->ai_addr, addr->ai_addrlen);
        rt->add_node(new Node(ip, port, weight, sock, addr));
    }
    printf("\n%s\n", id.c_str());
    void *sock_rt_id_int[] = {&lstn_sock, rt, &id, &interval};

    pthread_t pt_dbfrloop;
    pthread_create(&pt_dbfrloop, nullptr, dbfr_loop, (void *)sock_rt_id_int);

    char cmd[512];
    for(;;) {
        fgets(cmd, sizeof(cmd)-1, stdin);
        if(strcasecmp(cmd, "close\n") == FOUND) {
            break;
        }
        if(strcasecmp(cmd, "showrt\n") == FOUND) {
            rt->print_table();
        }
    }

    delete rt;
    return 0;
}
