//
// Created by Theodore Ahlfeld on 11/19/15.
//

#include "routing_table.h"

RoutingTable::RoutingTable(char *orig) {
    table = std::map<std::string, Node *>();
    neighbor_vec = std::map<std::string, double>();
    id = orig;
}

RoutingTable::~RoutingTable()
{
    for(auto it = table.begin(); it != table.end(); ++it) {

        delete it->second;
    }
    table.empty();
}

void RoutingTable::broadcast_all(const char *tuip_id) {
    for(auto it = table.begin(); it != table.end(); ++it) {
        Node *node = it->second;
        if(node->get_weight() == INFINITY) {
            continue;
        }
        if(node->timeout()) {
            node->set_weight(INFINITY);
        } else {
            double w = get_neighbor_weight(node);
            node->broadcast_to(tuip_id, w, &table, &neighbor_vec);
        }
    }
}

void RoutingTable::print_table_row(Node *node)
{
    if(node->get_weight() == INFINITY) {
        printf("## %-21s ## %-20s ## %-21s ##\n", node->get_alias(),
               "INFINITY", node->get_nearest_neighbor());
    } else {
        printf("## %-21s ## %-20.1f ## %-21s ##\n", node->get_alias(),
               node->get_weight()+get_node(node->get_nearest_neighbor())->get_neighbor_weight(),
               node->get_nearest_neighbor());
    }
    for(int i = 0; i < 76; i++) {
        putc('#', stdout);
    }
    putc('\n', stdout);
}

void RoutingTable::print_table()
{
    char buf[77];
    buf[76] = '\0';
    memset(buf, '#', sizeof(buf)-1);
    printf("%s\n## %-21s ## %-20s ## %-21s ##\n%s\n",  buf, "Node Dst",
           "Weight", "Node To Route Through", buf);
    for(auto it = table.begin(); it != table.end(); ++it) {
        print_table_row(it->second);
    }
    putc('\n', stdout);
}

Node *RoutingTable::get_node(const char *id) const
{
    auto it = table.find(std::string(id));
    if(it == table.end()) {
        return nullptr;
    }
    return it->second;
}

void RoutingTable::add_node(Node *node)
{
    table.insert(std::pair<std::string, Node *>(node->get_alias(), node));
}

void RoutingTable::remove_node(Node *node)
{
    table.erase(table.find(node->get_alias()));
    delete node;
}

const double RoutingTable::find_weight(char *id) const
{
    std::string s = id;
    auto it = table.find(s);
    if(it == table.end()) {
        return 0;
    }
    return it->second->get_weight();
}

void RoutingTable::add_neighbor(const Node *node)
{
    std::string s = node->get_alias();
    double w = node->get_weight();
    neighbor_vec.insert(std::pair<std::string, double>(s, w));
}

void RoutingTable::set_neighbor_weight(Node *node, const double w)
{
    node->set_neighbor_weight(w);
}

const double RoutingTable::get_neighbor_weight(Node *node) const
{

    std::string s = node->get_alias();
    auto it = neighbor_vec.find(s);
    if(it != neighbor_vec.end()) {
        return it->second;
    }
    return 0;
}