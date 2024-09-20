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

    // Enregistrer les murs
    for (auto* pObjet = _initData.objectInfoArray; pObjet != _initData.objectInfoArray + _initData.objectInfoArraySize; ++pObjet) {
        Point pointA{pObjet->q, pObjet->r};
        int hashA = pointA.calculerHash();
        int hashMur = Mur::calculerHash(hashA, pObjet->cellPosition);
        if (mapobjets.count(hashMur)) {
            // Mur déjà enregistré, ignorer
            continue;
        }
        
        if (!mapnoeuds.count(hashA)) {
            mapnoeuds.insert(std::pair<int, Noeud>(hashA, Noeud(pointA, TileType::Unknown)));
        }
        Noeud *noeudA = &mapnoeuds.at(hashA);
        Point pointB = noeudA->getPointNeighbour(pObjet->cellPosition);
        int hashB = pointB.calculerHash();
        if (!mapnoeuds.count(hashB)) {
            mapnoeuds.insert(std::pair<int, Noeud>(hashB, Noeud(pointB, TileType::Unknown)));
        }
        Noeud *noeudB = &mapnoeuds.at(hashB);

        mapobjets[hashMur] = {
            noeudA,
            noeudB,
            false
        };
        const Mur *mur = &mapobjets.at(hashMur);

        noeudA->addMur(mur);
        noeudB->addMur(mur);
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
