#pragma once

#include "Noeud.h"

struct SNoeudPotentiel {
	const Noeud* noeudPrecedent;
	int distanceDepart;
	int potentiel;
};

class AStar {
public:
    static std::vector<const Noeud*> calculerChemin(const Noeud *depart, const Noeud *arrivee);

private:
    static int calculerHeuristique(const Noeud *noeudA, const Noeud *noeudB)
    {
        return noeudA->point.calculerDistance(noeudB->point);
    }
};
