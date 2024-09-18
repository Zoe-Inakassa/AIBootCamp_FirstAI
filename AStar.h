#pragma once

#include "Noeud.h"

class AStar {
public:
    static std::vector<const Noeud*> calculerChemin(const Noeud *depart, const Noeud *arrivee);

private:
    static int calculerHeuristique(const Noeud *noeudA, const Noeud *noeudB)
    {
        // TODO
        // return noeudA->getDistanceVolOiseau(noeudB);
        return 0;
    }
};
