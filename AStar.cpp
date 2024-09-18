#include "AStar.h"
#include <map>
#include <set>

std::vector<const Noeud *> AStar::calculerChemin(const Noeud *depart, const Noeud *arrivee)
{
    std::map<const Noeud*, int> distancesDepart;
    std::map<const Noeud*, int> potentiels;
    std::map<const Noeud*, const Noeud*> parents;
    std::set<const Noeud*> noeudsAExplorer;

    distancesDepart[depart] = 0;
    potentiels[depart] = calculerHeuristique(depart, arrivee);
    noeudsAExplorer.insert(depart);

    // Exploration avec A*
    while (!noeudsAExplorer.empty() && !parents.count(arrivee)) {
        // Trouver le noeud avec le potentiel le plus faible
        int potentiel = INT_MAX;
        const Noeud *noeud = nullptr;
        for (auto noeudCandidat : noeudsAExplorer) {
            int potentielCandidat = potentiels.at(noeudCandidat);
            if (potentielCandidat < potentiel) {
                noeud = noeudCandidat;
                potentiel = potentielCandidat;
            }
        }

        // Retirer le noeud
        noeudsAExplorer.erase(noeud);

        // Explorer les voisins (un voisin est forcément accessible)
        for (auto voisin : noeud->getNeighbours()) {
            if (distancesDepart.count(voisin))
                continue;
            if (voisin->getTiletype() == TileType::Forbidden)
                continue;
            if (voisin->getTiletype() == TileType::Unknown)
                continue;

            distancesDepart[voisin] = distancesDepart[noeud] + 1;
            potentiels[voisin] = distancesDepart[voisin] + calculerHeuristique(voisin, arrivee);
            parents[voisin] = noeud;
            noeudsAExplorer.insert(voisin);
        }
    }

    // Construction du chemin
    std::vector<const Noeud*> cheminInverse;
    const Noeud *noeud = arrivee;
    while (noeud != depart) {
        cheminInverse.push_back(noeud);
        noeud = parents.at(noeud);
    }
    cheminInverse.push_back(depart);
    return cheminInverse;
}
