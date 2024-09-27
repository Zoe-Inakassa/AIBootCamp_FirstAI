#include "Dijkstra.h"

#include <algorithm>
#include <set>

#include "AStar.h"

std::vector<SNoeudDistance> Dijkstra::calculerDistances(const Noeud* depart, int distanceMax) {
    return calculerDistances(
        depart,
        [](const Noeud *noeud) { return noeud->getTiletype() == TileType::Goal; },
        distanceMax);
}

std::vector<SNoeudDistance> Dijkstra::calculerDistances(
    const Noeud* depart,
    std::function<bool(const Noeud*)> estObjectif,
    int distanceMax)
{
    std::vector<SNoeudDistance> noeudsAExplorer;
    std::set<const Noeud*> noeudsVu;
    std::vector<SNoeudDistance> goals;

    noeudsVu.insert(depart);
    noeudsAExplorer.push_back(SNoeudDistance{depart,0});
    if(estObjectif(depart)) goals.push_back(SNoeudDistance{depart,0});
    
    // Exploration avec Dijkstra
    int index = 0;
    while (index!=noeudsAExplorer.size() && noeudsAExplorer.at(index).distancedepart < distanceMax){
        // Explorer les voisins (un voisin est forcément accessible)
        int distance = noeudsAExplorer.at(index).distancedepart + 1;
        for (auto voisin : noeudsAExplorer.at(index).pnoeud->getNeighbours()) {
            // Si le noeud est déjà pris en compte ignorer
            if (noeudsVu.count(voisin))
                continue;
            if (voisin->getTiletype() == TileType::Forbidden)
                continue;
            if (voisin->getTiletype() == TileType::Unknown)
                continue;

            noeudsAExplorer.push_back(SNoeudDistance{voisin,distance});
            noeudsVu.insert(voisin);
            if (estObjectif(voisin)) goals.push_back(SNoeudDistance{voisin,distance});
        }
        
        // Retirer virtuellement le noeud que l'on vient d'explorer
        ++index;
    }
    
    return goals;
}
