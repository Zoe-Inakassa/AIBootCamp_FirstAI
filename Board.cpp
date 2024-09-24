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
    for (auto pObjet = _initData.objectInfoArray; pObjet != _initData.objectInfoArray + _initData.objectInfoArraySize; ++pObjet) {
        Point pointA{pObjet->q, pObjet->r};
        int hashA = pointA.calculerHash();
        int hashMur = Mur::calculerHash(hashA, pObjet->cellPosition);
        if(!existMur(hashMur))
        {
            addMur(*pObjet);
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

void Board::updateBoard(const STurnData &_turnData)
{
    for(int i=0; i!=_turnData.tileInfoArraySize; i++)
	{
		STileInfo tuile = _turnData.tileInfoArray[i];
		addTile(tuile);
	}
	
	for(int i=0; i!=_turnData.objectInfoArraySize; i++)
	{
		SObjectInfo object = _turnData.objectInfoArray[i];
		Point pointA{object.q, object.r};
		int hashA = pointA.calculerHash();
		int hashMur = Mur::calculerHash(hashA, object.cellPosition);
		if(!existMur(hashMur))
		{
			addMur(object);
		}
	}
}

void Board::addMur(const SObjectInfo& objet)
{
    Point pointA{objet.q, objet.r};
    int hashA = pointA.calculerHash();
    int hashMur = Mur::calculerHash(hashA, objet.cellPosition);
    if (!mapobjets.count(hashMur))
    {
        if (!mapnoeuds.count(hashA)) {
            mapnoeuds.insert(std::pair<int, Noeud>(hashA, Noeud(pointA, TileType::Unknown)));
        }
            
        Noeud *noeudA = &mapnoeuds.at(hashA);
        Point pointB = noeudA->getPointNeighbour(objet.cellPosition);
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
}

void Board::addTile(const STileInfo& tuile)
{
    Point point = Point{tuile.q, tuile.r};
    int hash = point.calculerHash();
    if(existNoeud(hash))
    {
        Noeud* noeud = getNoeud(hash);
        if(noeud->getTiletype() == TileType::Unknown)
        {
            if(tuile.type == EHexCellType::Default) noeud->setTiletype(TileType::Default);
            if(tuile.type == EHexCellType::Forbidden) noeud->setTiletype(TileType::Forbidden);
            if(tuile.type == EHexCellType::Goal) noeud->setTiletype(TileType::Goal);
        }else
        {
            return;
        }
    }else
    {
        if(tuile.type == EHexCellType::Default) mapnoeuds.insert(std::pair<int, Noeud>(hash, Noeud(point, TileType::Default)));
        if(tuile.type == EHexCellType::Forbidden) mapnoeuds.insert(std::pair<int, Noeud>(hash, Noeud(point, TileType::Forbidden)));
        if(tuile.type == EHexCellType::Goal) mapnoeuds.insert(std::pair<int, Noeud>(hash, Noeud(point, TileType::Goal)));
    }
    //TODO : ajouter les voisins (les murs sont déjà placés)
}
