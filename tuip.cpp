//
// Created by Theodore Ahlfeld on 12/3/15.
//

#include <cstring>
#include <stdlib.h>
#include "tuip.h"
#include "bf_node.h"

Tuip::Tuip(void *data)
{
    char buffer[512];
    char *base = (char *)data;
    char *field;
    char *end;
    size_t len;
    if((field = strtok(base, ":")) == nullptr) {
        goto err;
    }
    /* Copy IP */
    strncpy(buffer, field, sizeof(buffer));
    len = strlen(field);
    if((field = strtok(NULL, ":")) == nullptr) {
        goto err;
    }
    buffer[len++] = ':';
    /* Copy Port */
    strncat(buffer, field, sizeof(buffer)-len);
    len += strlen(field);
    strncpy(id, buffer, sizeof(id));

    /* Copy Weight */
    if((field = strtok(NULL, "\n")) == nullptr) {
        goto err;
    }
    weight = neighbor_weight = strtod(field, &end);

    while((field = strtok(nullptr, ":")) != nullptr && field[0] != '\n') {
        char *name = (char *)malloc(strlen(field)+1);
        strcpy(name, field);
        field = (strtok(nullptr, ":"));
        char *prt = (char *)malloc(strlen(field)+1);
        strcpy(prt, field);
        double w = strtod(strtok(nullptr, "\n"), &end);
        if(w == INFINITY) {
            ;
        }
        rt.push_back(Tuip(name, prt, w));
        free(name);
        free(prt);
    }
    base = (char *)data;
    if((end = strstr(base, "\n\n"))) {
        strcpy(base, end+2);
    } else {
        base[0] = '\0';
    }
    return;

    err:
    base[0] = '\0';
    return;
}

Tuip::Tuip(char *ip, char *port, const double w) {
    char buffer[512];
    strncpy(buffer, ip, sizeof(buffer));
    buffer[strlen(buffer)] = ':';
    strncat(buffer, port, sizeof(buffer)-strlen(ip)-1);
    strcpy(id, buffer);
    weight = w;

    if(w == INFINITY) {
        //printf("%s:%s->INFINITY\n", ip, port);
    } else {
        //printf("%s:%s->%f\n", ip, port, w);
    }
}

Tuip::~Tuip()
{
}