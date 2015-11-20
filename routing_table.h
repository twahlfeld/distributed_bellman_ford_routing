//
// Created by Theodore Ahlfeld on 11/19/15.
//

#ifndef __DBFR_TABLE_H
#define __DBFR_TABLE_H

#include <vector>
#include "bf_node.h"

class RoutingTable {
private:
    std::vector<Node *> table;

public:
    RoutingTable *showrt() {
        this;
    }

    RoutingTable() {

    }

    Node *add_node(const Node *node) {
        table.push_back(node);
    }

    ~RoutingTable();
};

#endif
