#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include <map>

#include "Noeud.h"


class Dijkstra{
public:
    static std::map<const Noeud*, int> calculerChemin(const Noeud* depart);
};

#endif