#include "Board.h"

Board::Board(): mapnoeuds{}, goals{}
{
    
}

void Board::initBoard(const SInitData& _initData)
{
    // Enregistrer les tiles pour les trouver rapidement
    for (auto* pTile = _initData.tileInfoArray; pTile != _initData.tileInfoArray + _initData.tileInfoArraySize; ++pTile) {
        // Enregistrer hash -> tile
        int hash = Point::calculerHash(pTile->q, pTile->r);
        mapnoeuds.insert(std::pair<int, Noeud>(hash, Noeud(*pTile)));

        // Enregistrer les goals
        if (pTile->type == EHexCellType::Goal) {
            goals.push_back(&mapnoeuds.at(hash));
        }
    }

    // Enregistrer les voisins
    for (auto& noeud : mapnoeuds) {
        std::vector<Point> points = noeud.second.point.surroundingPoints();
        for (auto& point : points)
        {
            auto adresse = mapnoeuds.find(point.calculerHash());
            if(adresse !=mapnoeuds.end())
            {
                noeud.second.addNeighbour(&(adresse->second));
            }
        }
    }
}
