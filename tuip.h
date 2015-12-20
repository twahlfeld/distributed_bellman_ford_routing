//
// Created by Theodore Ahlfeld on 12/3/15.
//

#ifndef __DBFR_TUIP_H
#define __DBFR_TUIP_H
#include <cstdio>
#include <string>
#include <vector>

class Tuip {
private:
    char id[22];
    double weight;
    double neighbor_weight;
    std::vector<Tuip> rt = std::vector<Tuip>();
public:
    Tuip(char *ip, char *port, const double w);
    Tuip(void *data);
    ~Tuip();
    std::vector<Tuip> *get_rt() { return &rt; }
    const double get_weight() const  { return weight; }
    const double get_neighbor() const { return neighbor_weight; }
    char *get_name() const { return (char *)id; }
    void set_weight(const double w) { weight = w; }
};


#endif //DISTRIBUTED_BELLMAN_FORD_ROUTING_TUIP_H
