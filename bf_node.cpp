//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include "bf_node.h"

#define NOFLAG 0

Node::Node(const char *ip_addr, const char *prt, const int64_t w,
           const int sock, addrinfo *addr) {
    ip = new string(ip_addr);
    port = new string(prt);
    weight = w;
    udp_sock = sock;
    udp_addr = addr;
    alias = string(ip_addr) + string(":") + string(prt);
    nearest_neighbor = alias;
}

void Node::broadcast_to(string *tuip_id,
                        std::map<string, Node *> *routemap)
{
    string tuip_msg;
    tuip_msg += *tuip_id + string("\n");
    for(auto it = routemap->begin(); it != routemap->end(); ++it) {
        Node *node = it->second;
        tuip_msg += node->get_alias() + string(":")
                 + std::to_string(node->get_weight()) + string("\n");
    }
    tuip_msg += string("\n");
    ssize_t len = sendto(udp_sock, tuip_msg.c_str(), tuip_msg.length(), NOFLAG,
                         udp_addr->ai_addr, udp_addr->ai_addrlen);
    printf("sent:%ld bytes\n", len);
}

void Node::update_route(Node *node)
{
    this->nearest_neighbor = node->get_alias();
    this->udp_sock = node->udp_sock;
    this->udp_addr = node->udp_addr;
}