#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include <map>
#include <functional>

#include "Noeud.h"

struct SNoeudDistance {
    const Noeud* pnoeud;
    int distancedepart;
};

class Dijkstra{
public:
    static std::vector<SNoeudDistance> calculerDistances(const Noeud* depart, int distanceMax=100);
    static std::vector<SNoeudDistance> calculerDistances(
        const Noeud* depart,
        std::function<bool(const Noeud*)> estObjectif,
        int distanceMax=100);
};

#endif