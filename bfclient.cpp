//
// Created by Theodore Ahlfeld on 11/19/15.
//

//#include "bfclient.h"
#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include "bf_node.h"
#include "routing_table.h"

void die_with_err(const char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        die_with_err("Invalid amount arguments");
    }
    uint8_t local_port = (uint8_t)atoi(argv[1]);
    time_t timeout = (time_t)atoi(argv[2]);
    RoutingTable *rt = new RoutingTable();
    int triplets = argc-2;
    if(argc%3) {
        die_with_err("Invalid triplet amount");
    }
    for(int trip_idx = 3; trip_idx < argc; trip_idx += 3) {
        rt->add_node(new Node(argv[trip_idx], (uint8_t)atoi(argv[trip_idx+1]),
                              (unsigned long long)(argv[trip_idx+2])));
    }
}