#include "Board.h"

Board::Board(): mapnoeuds{}, goals{}
{
    
}

void Board::initBoard(const SInitData& _initData)
{
    goalDecouvert = false;

    // Enregistrer les tiles pour les trouver rapidement
    for (auto* pTile = _initData.tileInfoArray; pTile != _initData.tileInfoArray + _initData.tileInfoArraySize; ++pTile) {
        addTile(*pTile);
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
    
    if (goalDecouvert) {
        calculerDistanceExplorationGoal();
    }
}

void Board::updateBoard(const STurnData &_turnData)
{
    goalDecouvert = false;
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

    if (goalDecouvert) {
        calculerDistanceExplorationGoal();
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
    TileType tiletype;
    switch (tuile.type) {
    case Default:
        tiletype = TileType::Default;
        break;
    case Goal:
        tiletype = TileType::Goal;
        break;
    case Forbidden:
        tiletype = TileType::Forbidden;
        break;
    default:
        throw ExceptionCellTypeInconnu{};
    }
    if(existNoeud(hash))
    {
        Noeud* noeud = getNoeud(hash);
        if(noeud->getTiletype() == TileType::Unknown)
        {
            noeud->setTiletype(tiletype);
        }
        else
        {
            return;
        }
    }else
    {
        mapnoeuds.insert(std::pair<int, Noeud>(hash, Noeud(point, tiletype)));
    }

    if (tiletype == TileType::Goal) {
        goalDecouvert = true;
    }

    //TODO : ajouter les voisins (les murs sont déjà placés)
    Noeud *noeud = getNoeud(hash);
    if (noeud->getTiletype() != TileType::Unknown) {
        for (Point pointVoisin : point.surroundingPoints()) {
            int hashVoisin = pointVoisin.calculerHash();
            if (!mapnoeuds.count(hashVoisin)) {
                mapnoeuds.insert(std::pair<int, Noeud>(hashVoisin, Noeud{ pointVoisin, TileType::Unknown }));
                Noeud *noeudVoisin = getNoeud(hash);
                noeudVoisin->addNeighbour(noeud);
                noeud->addNeighbour(noeudVoisin);
            }
        }
    }
}

void Board::calculerDistanceExplorationGoal()
{
    // Calculer les distances à vol d'oiseau des cases au goal le plus proche
    if (goalDecouvert) {
        for (auto& noeud : mapnoeuds) {
            // S'il y a un intérêt à explorer
            if (noeud.second.getNbVoisinsUnknown() > 0) {
                // Trouver la distance à vol d'oiseau avec le goal le plus proche
                int distanceGoalPlusProche = INT_MAX;
                for (Noeud *goal : goals) {
                    int distance = goal->point.calculerDistance(noeud.second.point);
                    if (distance < distanceGoalPlusProche) {
                        distanceGoalPlusProche = distance;
                    }
                }
                noeud.second.setDistanceVolGoal(distanceGoalPlusProche);
            }
        }
    }
}
