//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include "routing_table.h"
#include <math.h>

RoutingTable::~RoutingTable()
{
    table.empty();
}

void RoutingTable::broadcast_all(string *tuip_id) {
    for(auto it = table.begin(); it != table.end(); ++it) {
        it->second->broadcast_to(tuip_id, &table);
    }
}

void print_table_row(Node *node)
{
    printf("# %-21s # %-20lld # %-21s #\n", node->get_alias().c_str(),
           node->get_weight(), node->get_nearest_neighbor().c_str());
    for(int i = 0; i < 72; i++) {
        putc('#', stdout);
    }
    putc('\n', stdout);
}

void RoutingTable::print_table()
{
    char buf[72];
    memset(buf, '#', sizeof(buf));
    printf("%s\n# %-21s # %-20s # %-21s #\n%s\n",  buf, "Node Dst",
           "Weight", "Node To Route Through", buf);
    for(auto it = table.begin(); it != table.end(); ++it) {
        print_table_row(it->second);
    }
    putc('\n', stdout);
}