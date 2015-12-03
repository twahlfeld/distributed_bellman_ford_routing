//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include "routing_table.h"

RoutingTable::~RoutingTable()
{
    table.empty();
}

void RoutingTable::broadcast_all(string *tuip_id) {
    for(auto it = table.begin(); it != table.end(); ++it) {
        it->second->broadcast_to(tuip_id, &table);
    }
}