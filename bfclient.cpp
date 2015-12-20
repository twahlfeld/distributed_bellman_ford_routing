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
#include <err.h>
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

addrinfo *create_udp_addr(const char *hostname, const char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;
    struct addrinfo *addr = nullptr;
    int error;
    if((error = getaddrinfo(hostname, port, &hints, &addr))) {
        perror("getaddrinfo() failed");
        errx(1, "%s", gai_strerror(error));
    }
    return addr;
}

int create_udp_socket(addrinfo *addr)
{
    int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < -1) {
        die_with_err("socket() failed");
    }
    return sock;
}

void bind_udp(int sock, struct addrinfo *addr)
{
    if (bind(sock, addr->ai_addr, addr->ai_addrlen) == -1) {
        die_with_err("bind() failed");
    }
}

int open_udp_listening_socket(const char *port)
{
    const int enabled = 1;
    struct addrinfo *addr = create_udp_addr(nullptr, port);
    if(!addr) {
        return 0;
    }
    int sock = create_udp_socket(addr);
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enabled, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEPORT) failed");
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

char *tuip_id(const char *port)
{
    int sock;
    struct ifreq ifr;
    size_t len;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, IPV4INTERFACE, IFNAMSIZ-1);
    ioctl(sock, SIOCGIFADDR, &ifr);
    close(sock);
    char *ip = inet_ntoa(((sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    char *id = (char *)malloc(((len=(strlen(ip)))+strlen(port)+2)*sizeof(char));
    strcpy(id, ip);
    id[len++] = ':';
    strcpy(id+len, port);
    return id;
}

void update_adj_vec(Tuip *tuip, RoutingTable *rt, const time_t to,
                    const double upd_weight)
{
    Node *node = rt->get_node(tuip->get_name());
    if (node == nullptr) {
        return;
    }
    if(tuip->get_weight() == INFINITY) {
        ;
    }
    std::vector<Tuip> *tuip_vec = tuip->get_rt();
    double cur_w = upd_weight;
    double neighbor_w = node->get_neighbor_weight();
    //double past_w = rt->find_weight(tuip->get_name());
    //double cur_nghbr_w = tuip->get_neighbor();
    //double past_nghbr_w = rt->get_neighbor_weight(node);
    node->set_neighbor_weight(tuip->get_weight());
    //double diff = cur_w - past_w;
    //double nbghbr_diff = cur_nghbr_w - past_nghbr_w;
    for(auto it = tuip_vec->begin(); it != tuip_vec->end(); ++it) {
        Node *dst = rt->get_node(it->get_name());
        if(!dst) { // Not found
            if(strcmp(rt->get_id(), it->get_name())) {
                char *base = strtok(it->get_name(), ":");
                char *ip, *port;
                strcpy(ip = (char *) malloc(strlen(base) + 1), base);
                base = strtok(nullptr, ":\n\0");
                strcpy(port = (char *) malloc(strlen(base) + 1), base);
                Node *tmp = new Node(ip, port, 0, neighbor_w, to,
                                     node->get_sock(), node->get_addr(),
                                     tuip->get_name());
                rt->add_node(tmp);
                rt->add_neighbor(tmp);
            }
        } else { // Node found
            double totalw = dst->get_neighbor_weight() + dst->get_weight();
            //dst->set_neighbor_weight(tuip->get_weight());
            if(dst->get_weight() == INFINITY && it->get_weight() != INFINITY) {
                dst->link_up();
            }
            /* nearest neighbor is itself */
            if(strcmp(node->get_nearest_neighbor(), node->get_alias())){
                node->set_neighbor_weight(tuip->get_weight());
            }
            if(totalw > neighbor_w + cur_w) {
                dst->update_route(node);
                dst->set_weight(cur_w+neighbor_w);
            }
        }
    }
}

void *dbfr_loop(void *kludged_data) {
    /* Variable Initialization */
    void **sock_rt_id_int_loop = (void **)kludged_data;
    int lstn_sock = *(int *) sock_rt_id_int_loop[0];
    RoutingTable *rt = (RoutingTable *) sock_rt_id_int_loop[1];
    const char *id = (char *) sock_rt_id_int_loop[2];
    time_t *interval = (time_t *) sock_rt_id_int_loop[3];
    int *loop = (int *) sock_rt_id_int_loop[4];
    sockaddr_storage src_addr;
    socklen_t src_len = sizeof(src_addr);
    char buffer[4096];
    ssize_t len;

    /* Main DBFR loop */
    while (*loop) {
        if (timer_send(interval)) { // Check send interval
            rt->broadcast_all(id);
        }

        /* Socket recvfrom logic */
        if((len = recvfrom(lstn_sock, buffer, sizeof(buffer)-1,
                           0, (sockaddr *)&src_addr, &src_len)) < 0 &&
                           errno == EAGAIN) {
            // no data to read
        } else if(len > 0) {
            //printf("recv:\n%s\n", buffer);
            double weight;
            char *end;
            weight = strtod(strstr(buffer, id)+strlen(id)+1, &end);

            if(weight == INFINITY) {
            ;//    printf("NEWEST WEIGHT:%s\n", "INFINITY");
            } else {
            //    printf("NEWEST WEIGHT:%f\n", weight);
            }


            buffer[len] = '\0';
            while(buffer[0] != '\0') {
                Tuip tuip = Tuip(buffer);
                Node *node = rt->get_node(tuip.get_name());
                if(node) {
                    if(tuip.get_weight() != INFINITY) {
                        node->update_broadcast_time();
                        update_adj_vec(&tuip, rt, *interval, weight);
                    }
                } else {
                    char *base = strtok(tuip.get_name(), ":");
                    char *ip, *port;
                    strcpy(ip = (char *)malloc(strlen(base)+1), base);
                    base = strtok(nullptr, ":\n\0");
                    strcpy(port = (char *)malloc(strlen(base)+1), base);
                    addrinfo *addr = create_udp_addr(ip, port);
                    int sock = create_udp_socket(addr);
                    Node *tmp = new Node(ip, port, 0, weight, *interval,
                                         sock, addr, nullptr);
                    rt->add_node(tmp);
                    rt->add_neighbor(tmp);
                }
            }
        } else { // ERROR
            perror("recvfrom() failed");
            *loop = 0;
            break;
        }

    }
    return nullptr;
}

void *cmd_loop(void *kludged_data)
{
    void **id_rt_loop = (void **)kludged_data;
    char *id = (char *)id_rt_loop[0];
    RoutingTable *rt = (RoutingTable *) id_rt_loop[1];
    int *loop = (int *) id_rt_loop[2];

    char cmd[512];
    ssize_t len;
    while(*loop) {
        int in_fd = fileno(stdin);
        if ((len = read(in_fd, cmd, sizeof(cmd) - 1)) < 0 && errno == EAGAIN) {
            // no data to read
        } else if (len > 0) {
            cmd[len-1] = '\0';
            if(strcasecmp(cmd, "close") == FOUND) {
                break;
            }
            if(strcasecmp(cmd, "showrt") == FOUND) {
                rt->print_table();
            }
            if(strncasecmp(cmd, "linkdown ", 9) == FOUND) {
                Node *node = rt->get_node(cmd+9);
                if(node) {
                    node->link_down();
                    rt->set_neighbor_weight(node, INFINITY);
                    /*node->broadcast_to(id, INFINITY,
                                       rt->get_table(), rt->get_nghbr_map());*/
                }
                //strcpy(cmd, id);
                rt->broadcast_all(id);
            }
        } else { // Error Occurred
            break;
        }
    }
    return nullptr;
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        die_with_err("Invalid amount arguments");
    }
    const char *local_port = argv[1];
    time_t interval = (time_t)atoi(argv[2]);
    char *id = tuip_id(local_port);
    RoutingTable *rt = new RoutingTable(id);
    if(argc%3) {
        die_with_err("Invalid triplet amount");
    }
    int lstn_sock = open_udp_listening_socket(local_port);
    if(!lstn_sock) {
        die_with_err("Listen Socket failed");
    }
    for(int trip_idx = 3; trip_idx < argc; trip_idx += 3) {
        char *ip = (char *) malloc(strlen(argv[trip_idx]) + 1);
        strcpy(ip, argv[trip_idx]);
        char *port = (char *) malloc(strlen(argv[trip_idx + 1]) + 1);
        strcpy(port, argv[trip_idx + 1]);
        double weight = strtod(argv[trip_idx + 2], nullptr);
        addrinfo *addr = create_udp_addr(ip, port);
        if(!addr) {
            die_with_err(ip);
        }
        int sock = create_udp_socket(addr);
        Node *node = new Node(ip, port, 0, weight,
                              interval, sock, addr, nullptr);
        rt->add_node(node);
        rt->add_neighbor(node);
    }
    //printf("\n%s\n", id);

    pthread_t pt_dbfrloop;
    pthread_t pt_cmdloop;
    int loop = 1;
    void *sock_rt_id_int_loop[] = {&lstn_sock, rt, id, &interval, &loop};
    void *id_rt_loop[] = {id, rt, &loop};
    pthread_create(&pt_dbfrloop, nullptr, dbfr_loop, sock_rt_id_int_loop);
    pthread_create(&pt_cmdloop, nullptr, cmd_loop, id_rt_loop);
    pthread_join(pt_cmdloop, NULL);
    pthread_join(pt_dbfrloop, NULL);
    free(id);
    delete rt;
    return 0;
}
