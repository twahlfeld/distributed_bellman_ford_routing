//
// Created by Theodore Ahlfeld on 11/19/15.
//

#ifndef __DBFR_NODE_H
#define __DBFR_NODE_H

#include <cstdint>

class Node {
private:
    char *IPaddr;
    uint8_t  port;
    unsigned long long weight;
public:
    Node(char *ip, uint8_t prt, unsigned long long w) {
        IPaddr = ip;
        port = prt;
        weight = w;
    }
    ~Node() {
        if(IPaddr) {
            delete IPaddr;
        }
    }
    char *get_ip() {
        return IPaddr;
    }
    uint8_t get_port() {
        return port;
    }
    unsigned long long get_weight() {
        weight;
    }

};

#endif
