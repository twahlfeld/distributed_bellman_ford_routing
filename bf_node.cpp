//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include <cstring>
#include "bf_node.h"

#define NOFLAG 0

Node::Node(char *ip_addr, char *prt, const double w, const double nw,
           const time_t to, const int sock, addrinfo *addr, char *neighbor) {
    /* Member Data Initiliaztion */
    time(&last_broadcast);
    timeout_val = to;
    size_t len;
    ip = ip_addr;
    port = prt;
    last_weight = neighbor_weight = nw;
    weight = w;
    udp_sock = sock;
    udp_addr = addr;

    /* Logic */
    char buffer[512];
    strncpy(buffer, ip, sizeof(buffer));
    if((len = strlen(ip)) >= sizeof(buffer)) goto err;
    buffer[len++] = ':';
    buffer[len] = '\0';
    strncat(buffer, prt, sizeof(buffer)-len);
    alias = (char *)malloc(strlen(buffer)*sizeof(char)+1);
    strcpy(alias, buffer);
    nearest_neighbor = (neighbor ? neighbor : alias);
    return;
    err:
        free(ip);
        free(port);
        delete this;
}

Node::~Node()
{
    free(ip);
    free(port);
    free(alias);
    freeaddrinfo(udp_addr);
}

bool Node::timeout()
{
    time_t tv_now;
    if(time(&tv_now) > (timeout_val * 100) + last_broadcast) {
        return 1;
    }
    return 0;
}

void Node::broadcast_to(const char *tuip_id, double nghbr_w,
                        std::map<std::string, Node *> *routemap,
                        std::map<std::string, double> *nghbr)
{
    char buffer[4096];
    size_t len;
    strncpy(buffer, tuip_id, sizeof(buffer));
    nghbr_w = this->get_neighbor_weight();

    std::string neighbor_w;
    if(fabs(nghbr_w) == INFINITY) {
        neighbor_w = std::string("INFINITY");
    } else {
        neighbor_w = std::to_string(nghbr_w);
    }
    snprintf(buffer, sizeof(buffer), "%s:%s\n", tuip_id, neighbor_w.c_str());
    len = strlen(buffer);
    //printf("%s", buffer);
    char *alais;
    for(auto it = routemap->begin(); it != routemap->end(); ++it) {
        Node *node = it->second;
        strncat(buffer, alais = node->get_alias(), sizeof(buffer)-(len));
        len+=strlen(alais);
        buffer[len++] = ':';
        double w = node->get_weight();
        std::string weight;
        if(fabs(w) == INFINITY) {
            weight = std::string("INFINITY");
        } else {
            weight = std::to_string(w);
        }
        if(weight == "nan") {
            goto err;
        }
        len += weight.length();
        strncat(buffer, weight.c_str(), sizeof(buffer)-len);
        buffer[len++] = '\n';
    }
    buffer[len] = '\n';
    sendto(udp_sock, buffer, len, NOFLAG,
           udp_addr->ai_addr, udp_addr->ai_addrlen);
    printf("sent:%ld bytes to:%s\n", len, this->get_alias());
    printf("%s\n", buffer);
    return;

    err:
        printf("FOUND NAN %s:%f\n", get_alias(), get_weight());

}

void Node::update_route(Node *node)
{
    this->nearest_neighbor = node->get_alias();
    //this->neighbor_weight = node->get_neighbor_weight();
    /* Probably don't need this
     * this->udp_sock = node->get_sock();
     * this->udp_addr = node->get_addr();*/
}

void Node::link_down() {
    last_weight = weight;
    weight = INFINITY;
}