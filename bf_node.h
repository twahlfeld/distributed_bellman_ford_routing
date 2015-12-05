//
// Created by Theodore Ahlfeld on 11/19/15.
//

#ifndef __DBFR_NODE_H
#define __DBFR_NODE_H

#include <cstdint>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <map>
#include <string>

#define INFINITY -1

using std::string;

class Node {
private:
    string *ip;
    string *port;
    long long weight;
    int udp_sock;
    addrinfo *udp_addr;
    //std::map<std::string, Node *> adjlist;
    string alias;
    time_t last_broadcast;
    string nearest_neighbor;

public:
    Node(const char *ip_addr, const char *prt, const int64_t w,
         const int sock, addrinfo *addr);
    ~Node()
    {
        delete ip;
        delete port;
        freeaddrinfo(udp_addr);
    }

    string get_nearest_neighbor()
    {
        return nearest_neighbor;
    }

    std::string get_alias() const
    {
        return alias;
    }

    long long get_weight() const
    {
        return weight;
    }

    void broadcast_to(string *tuip_id, std::map<string, Node *> *routemap);

    bool timeout(time_t *timeout) {
        time_t tv_now;
        return time(&tv_now) > *timeout * 3 + last_broadcast;
    }

    void update_route(Node *node);
    void set_weight(long long w)
    {
        weight = w;
    }
};

#endif
