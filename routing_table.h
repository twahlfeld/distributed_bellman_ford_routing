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
    char *id;
    std::map<std::string, Node *> table;
    std::map<std::string, double> neighbor_vec;
    void print_table_row(Node *node);

public:
    RoutingTable(char *origin);
    ~RoutingTable();

    /* Accessors */
    std::map<std::string, Node *> *showrt() { return &table; }
    std::map<std::string, Node *> *get_table() { return &table; }
    std::map<std::string, double> *get_nghbr_map() { return &neighbor_vec; }
    const std::map<std::string,Node *>::iterator begin(){return table.begin();}
    const std::map<std::string, Node *>::iterator end() { return table.end(); }
    const char *get_id() const { return id; }
    Node *get_node(const char *id) const;
    const double find_weight(char *id) const;
    const double get_neighbor_weight(Node *node) const;

    /* Mutators */
    void set_neighbor_weight(Node *node, const double w);
    void add_node(Node *node);
    void add_neighbor(const Node *node);
    void remove_node(Node *node);
    void print_table();
    void broadcast_all(const char *tuip_id);
};

#endif
