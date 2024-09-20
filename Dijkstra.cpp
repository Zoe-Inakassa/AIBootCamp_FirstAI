#include "Dijkstra.h"

#include <algorithm>
#include <set>

#include "AStar.h"

std::vector<SNoeudDistance> Dijkstra::calculerDistances(const Noeud* depart, int distanceMax)
{
    std::vector<SNoeudDistance> noeudsAExplorer;
    std::set<const Noeud*> noeudsVu;
    std::vector<SNoeudDistance> goals;

    noeudsVu.insert(depart);
    noeudsAExplorer.push_back(SNoeudDistance{depart,0});
    if(depart->getTiletype() == TileType::Goal) goals.push_back(SNoeudDistance{depart,0});
    
    // Exploration avec Dijkstra
    auto pointeur = noeudsAExplorer.begin();
    while (pointeur!=noeudsAExplorer.end() && pointeur->distancedepart < distanceMax){
        // Explorer les voisins (un voisin est forcément accessible)
        int distance = pointeur->distancedepart + 1;
        for (auto voisin : pointeur->pnoeud->getNeighbours()) {
            // Si le noeud est déjà pris en compte ignorer
            if (noeudsVu.count(voisin))
                continue;
            if (voisin->getTiletype() == TileType::Forbidden)
                continue;
            if (voisin->getTiletype() == TileType::Unknown)
                continue;

            noeudsAExplorer.push_back(SNoeudDistance{voisin,distance});
            noeudsVu.insert(voisin);
            if (voisin->getTiletype() == TileType::Goal) goals.push_back(SNoeudDistance{voisin,distance});
        }
        
        // Retirer virtuellement le noeud que l'on vient d'explorer
        ++pointeur;
    }
    
    return goals;
}
