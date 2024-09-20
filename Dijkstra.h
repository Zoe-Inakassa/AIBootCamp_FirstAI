#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include <map>

#include "Noeud.h"

struct SNoeudDistance {
    const Noeud* pnoeud;
    int distancedepart;
};

class Dijkstra{
public:
    static std::vector<SNoeudDistance> calculerDistances(const Noeud* depart, int distanceMax=100);
};

#endif