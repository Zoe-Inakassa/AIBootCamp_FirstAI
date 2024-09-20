#include "AStar.h"
#include <map>
#include <set>
#include <algorithm>

std::vector<const Noeud *> AStar::calculerChemin(const Noeud *depart, const Noeud *arrivee)
{
    std::vector<const Noeud*> noeudsAExplorer;
    std::map<const Noeud*, SNoeudPotentiel> potentiels;

    potentiels[depart] = SNoeudPotentiel{
        nullptr, // Parent
        0, // Distance
        calculerHeuristique(depart, arrivee) // Potentiel
    };
    noeudsAExplorer.push_back(depart);

    // Exploration avec A*
    while (!noeudsAExplorer.empty() && !potentiels.count(arrivee)) {
        // Trier par potentiel décroissant
        std::sort(noeudsAExplorer.begin(), noeudsAExplorer.end(), [&potentiels](const Noeud *a, const Noeud *b) {
            return potentiels.at(a).potentiel > potentiels.at(b).potentiel;
        });
        // Noeud avec le potentiel le plus faible
        const Noeud *noeud = noeudsAExplorer.back();
        const SNoeudPotentiel &infoNoeud = potentiels.at(noeud);

        // Retirer le noeud
        noeudsAExplorer.pop_back();

        // Explorer les voisins (un voisin est forcément accessible)
        int distance = infoNoeud.distanceDepart + 1;
        for (auto voisin : noeud->getNeighbours()) {
            // Si le noeud existe et que sa distance au départ est meilleure, ignorer
            if (potentiels.count(voisin) && potentiels.at(voisin).distanceDepart <= distance)
                continue;
            if (voisin->getTiletype() == TileType::Forbidden)
                continue;
            if (voisin->getTiletype() == TileType::Unknown)
                continue;

            if (!potentiels.count(voisin))
                noeudsAExplorer.push_back(voisin);
            potentiels[voisin] = {
                noeud, // Parent
                distance, // Distance
                distance + calculerHeuristique(voisin, arrivee) // Potentiel
            };
        }
    }

    // Construction du chemin
    std::vector<const Noeud*> cheminInverse;
    if (potentiels.count(arrivee)) {
        const Noeud *noeud = arrivee;
        while (noeud != depart) {
            cheminInverse.push_back(noeud);
            noeud = potentiels.at(noeud).noeudPrecedent;
        }
        cheminInverse.push_back(depart);
    }
    return cheminInverse;
}
