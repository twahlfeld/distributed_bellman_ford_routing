//
// Created by Theodore Ahlfeld on 11/19/15.
//

#ifndef __DBFR_NODE_H
#define __DBFR_NODE_H

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <float.h>
#include <string>
#include <map>
#include <vector>

//#define INFINITY DBL_MAX

class Node {
private:
    char *ip;
    char *port;
    double weight;
    double last_weight;
    double neighbor_weight;
    int udp_sock;
    addrinfo *udp_addr;
    char *alias;
    time_t last_broadcast;
    char *nearest_neighbor;
    time_t timeout_val;

public:
    Node(char *ip_addr, char *prt, const double w, const double nw,
         const time_t to, const int sock, addrinfo *addr, char *neighbor);
    ~Node();

    /* Accessors */
    char *get_nearest_neighbor() const { return nearest_neighbor; }
    char *get_alias() const { return alias; }
    const int get_sock() const { return udp_sock; }
    addrinfo *get_addr() const { return udp_addr; }
    const double get_weight() const { return weight; }
    const double get_neighbor_weight() const { return neighbor_weight; }
    bool timeout();

    /* Mutators */
    void set_neighbor_weight(const double w) { neighbor_weight = w; }
    void set_weight(const double w) { weight = w; }
    void update_broadcast_time() { time(&last_broadcast); }
    void link_down();
    void link_up() { weight = last_weight; }
    void broadcast_to(const char *tuip_id, double nghbr_w,
                      std::map<std::string, Node *> *routemap,
                      std::map<std::string, double> *nghbr);
    void update_route(Node *node);
};

#endif
