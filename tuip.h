//
// Created by Theodore Ahlfeld on 12/3/15.
//

#ifndef __DBFR_TUIP_H
#define __DBFR_TUIP_H
#include <cstdio>
#include <string>
#include <vector>

using std::string;

class Tuip {
private:
    string id;
    long long weight;
    std::vector<Tuip> *rt;
public:
    Tuip(string alias, long long w) {
        id = alias;
        weight = w;
        rt = nullptr;
    }
    Tuip(uint8_t *data);
    ~Tuip();
    void set_weight(long long w)
    {
        weight = w;
    }

    std::vector<Tuip> *get_rt()
    {
        return rt;
    }

    long long get_weight()
    {
        return weight;
    }

    string get_name()
    {
        return id;
    }
};


#endif //DISTRIBUTED_BELLMAN_FORD_ROUTING_TUIP_H
