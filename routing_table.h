//
// Created by Theodore Ahlfeld on 11/19/15.
//

#ifndef __DBFR_TABLE_H
#define __DBFR_TABLE_H

#include <vector>
#include <cstring>
#include "bf_node.h"

class RoutingTable {
private:
    std::map<string, Node *>  table;

public:
    std::map<string, Node *> *showrt() {
        return &table;
    }

    std::map<string, Node *> *get_table() {
        return &table;
    };

    RoutingTable() {

    }

    void add_node(Node *node) {
        table.insert(std::pair<string, Node *>(node->get_alias(), node));
    }

    void remove_node(Node *node) {
        table.erase(table.find(node->get_alias()));
        delete node;
    }

    Node *get_node(string id)
    {
        if(table.find(id) == table.end()) {
            return nullptr;
        }
        return table.find(id)->second;
    }

    long long find_weight(string id) {
        return table.find(id)->second->get_weight();
    }

    void print_table();

    void broadcast_all(string *tuip_id);

    ~RoutingTable();
};

#endif
