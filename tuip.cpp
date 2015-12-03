//
// Created by Theodore Ahlfeld on 12/3/15.
//

#include <cstring>
#include <stdlib.h>
#include "tuip.h"

Tuip::Tuip(uint8_t *data)
{
    char *buf = (char *)data;
    rt = new std::vector<Tuip>;
    char *field;
    char *end;
    if((field = strtok(nullptr, ":")) != nullptr) {
        id = string(field);
    } else {
        goto err;
    }
    if((field = strtok(NULL, "\n")) != nullptr) {
        id += string(field);
    } else {
        goto err;
    }
    weight = 0;
    while((field = strtok(nullptr, ":")) != nullptr && field[0] != '\n') {
        string name = string(field);
        string prt = string(strtok(buf, ":"));
        long long w = strtoll(string(field=strtok(buf, "\n")).c_str(),&end,10);
        if(w == 0) {
            goto err;
        }
        rt->push_back(Tuip(name + prt, w));
    }
    if((end = strstr(buf, "\n\n"))) {
        strcpy(buf, end+2);
    } else {
        buf[0] = '\0';
    }
    return;

    err:
    delete this;
}

Tuip::~Tuip()
{
    delete rt;
}