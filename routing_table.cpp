//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include "routing_table.h"

RoutingTable::~RoutingTable()
{
    while(!table.empty()) {
        delete table.back();
        table.pop_back();
    }
}