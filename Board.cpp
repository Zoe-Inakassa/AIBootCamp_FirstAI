#include "Board.h"

Board::Board(): mapnoeuds{}, goals{}, nombreTileMaxDroite{INT_MAX}, nombreTileMaxBas{INT_MAX}
{
    
}

Board::~Board()
{
    for (auto it : mapnoeuds) {
        delete it.second;
    }
    mapnoeuds.clear();
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
        std::vector<Point> points = noeud.second->point.surroundingPoints();
        for (auto& point : points)
        {
            auto adresse = mapnoeuds.find(point.calculerHash());
            if(adresse !=mapnoeuds.end())
            {
                noeud.second->addNeighbour(adresse->second);
            }
        }
    }
    
    if (goalDecouvert) {
        calculerDistancesGoalsTousNoeuds();
    }
}

void Board::updateBoard(const STurnData &_turnData, const std::vector<NPC> &listeNPC)
{
    goalDecouvert = false;
    for(int i=0; i!=_turnData.tileInfoArraySize; i++)
	{
		STileInfo tuile = _turnData.tileInfoArray[i];
		addTile(tuile);
        int hash = Point::calculerHash(tuile.q,tuile.r);
        calculerDistancesGoalsUnNoeud(mapnoeuds.at(hash));
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

    calculerBordures(listeNPC);

    if (goalDecouvert) {
        calculerDistancesGoalsTousNoeuds();
    }
}

void Board::addMur(const SObjectInfo& objet)
{
    Point pointA{objet.q, objet.r};
    int hashA = pointA.calculerHash();
    int hashMur = Mur::calculerHash(hashA, objet.cellPosition);
    if (!mapobjets.count(hashMur))
    {
        // Obtenir ou créer le noeud
        auto it = mapnoeuds.find(hashA);
        Noeud *noeudA;
        if (it == mapnoeuds.end()) {
            noeudA = mapnoeuds[hashA] = new Noeud(pointA, TileType::Unknown);
        } else {
            noeudA = (*it).second;
        }
        
        // Obtenir ou créer le noeud
        Point pointB = noeudA->getPointNeighbour(objet.cellPosition);
        int hashB = pointB.calculerHash();
        it = mapnoeuds.find(hashB);
        Noeud *noeudB;
        if (it == mapnoeuds.end()) {
            noeudB = mapnoeuds[hashB] = new Noeud(pointB, TileType::Unknown);
        } else {
            noeudB = (*it).second;
        }

        bool transparent = false;
        for (int *type = objet.types; type != objet.types + objet.typesSize; ++type) {
            if (*type == EObjectType::Window)
                transparent = true;
        }

        mapobjets[hashMur] = {
            noeudA,
            noeudB,
            transparent
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
    Noeud *noeud;
    if(existNoeud(hash))
    {
        noeud = getNoeud(hash);
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
        noeud = mapnoeuds[hash] = new Noeud(point, tiletype);
    }

    if (tiletype == TileType::Goal) {
        goalDecouvert = true;
        goals.insert(noeud);
    }

    if (tiletype != TileType::Unknown) {
        // Créer des voisins fictifs qui peuvent être en dehors de la carte
        for (Point pointVoisin : point.surroundingPoints()) {
            if (!pointEstPossible(pointVoisin)) {
                // Ce voisin ne peut pas exister
                continue;
            }

            int hashVoisin = pointVoisin.calculerHash();
            auto itVoisin = mapnoeuds.find(hashVoisin);
            Noeud *noeudVoisin;
            if (itVoisin == mapnoeuds.end()) {
                noeudVoisin = mapnoeuds[hashVoisin] = new Noeud{ pointVoisin, TileType::Unknown };
            } else {
                noeudVoisin = itVoisin->second;
            }
            if (noeudVoisin->getTiletype() == TileType::Unknown) {
                noeudVoisin->addNeighbour(noeud);
                noeud->addNeighbour(noeudVoisin);
            }
        }
    }
}

bool Board::pointEstPossible(Point point) const
{
    int x = point.q + 2 * point.r;
    // y = point.q
    if (point.q < 0 || nombreTileMaxBas < point.q) {
        // Le point est plus haut que 0,0 ou plus bas que le noeud le plus bas autorisé
        return false;
    }
    if (x < 0 || nombreTileMaxDroite < x) {
        // Le point est à gauche de 0,0 ou plus à droite que le noeud le plus à droite autorisé
        return false;
    }
    // Le point existe peut-être, d'aprés les informations actuelles.
    return true;
}

void Board::calculerDistancesGoalsTousNoeuds()
{
    // Calculer les distances à vol d'oiseau des cases au goal le plus proche
    for (auto& noeud : mapnoeuds) {
        // S'il y a un intérêt à explorer
        calculerDistancesGoalsUnNoeud(noeud.second);
    }
}

void Board::calculerDistancesGoalsUnNoeud(Noeud* noeud)
{
    if (noeud->getNbVoisinsUnknown() > 0) {
        // Trouver la distance à vol d'oiseau avec le goal le plus proche
        int distanceGoalPlusProche = INT_MAX;
        if (goals.empty()) {
            // Le goal est peut être à côté
            distanceGoalPlusProche = 1;
        } else {
            for (Noeud *goal : goals) {
                int distance = goal->point.calculerDistance(noeud->point);
                if (distance < distanceGoalPlusProche) {
                    distanceGoalPlusProche = distance;
                }
            }
        }
        noeud->setDistanceVolGoal(distanceGoalPlusProche);
    }
}

void Board::calculerBordures(const std::vector<NPC> &listeNPC)
{
    // Si un NPC pourrait voir une tile mais que celle-ci n'existe pas ou est UNKNOWN
    // Alors, nous sommes à la bordure
    // La bordure gauche et haute sont définies par 0,0
    bool bordureChangee = false;

    // Trouver la bordure droite
    if (nombreTileMaxDroite == INT_MAX) {
        for (const NPC &npc : listeNPC) {
            const Noeud *emplacement = npc.getEmplacement();
            // TODO: on suppose pour l'instant une vision de 1

            // S'il n'y a pas de mur pour bloquer la vue
            if (!emplacement->hasOpaqueMur(EHexCellDirection::E)) {
                int hashNoeudE = emplacement->getPointNeighbour(EHexCellDirection::E).calculerHash();
                const Noeud *noeudE = mapnoeuds.count(hashNoeudE) ? getNoeud(hashNoeudE) : nullptr;
                // Les noeuds voisins existent forcément, sauf cas de bordure
                if (noeudE == nullptr || noeudE->getTiletype() == TileType::Unknown) {
                    int xMax = emplacement->point.q + 2 * emplacement->point.r;
                    if (emplacement->point.q % 2 == 0) {
                        // Cas d'un q pair : xMax est 1 plus à droite sur les lignes impaires
                        ++xMax;
                    }
                    nombreTileMaxDroite = xMax;
                    bordureChangee = true;
                    break;
                }
            }
        }
    }

    // Trouver la bordure basse
    if (nombreTileMaxBas == INT_MAX) {
        for (const NPC &npc : listeNPC) {
            const Noeud *emplacement = npc.getEmplacement();
            // TODO: on suppose pour l'instant une vision de 1

            // S'il n'y a pas de mur pour bloquer la vue
            if (!emplacement->hasOpaqueMur(EHexCellDirection::SE)) {
                int hashNoeudSE = emplacement->getPointNeighbour(EHexCellDirection::SE).calculerHash();
                const Noeud *noeudSE = mapnoeuds.count(hashNoeudSE) ? getNoeud(hashNoeudSE) : nullptr;
                // Les noeuds voisins existent forcément, sauf cas de bordure
                if (noeudSE == nullptr || noeudSE->getTiletype() == TileType::Unknown) {
                    nombreTileMaxBas = emplacement->point.q;
                    bordureChangee = true;
                    break;
                }
            }
        }
    }

    if (bordureChangee) {
        // Retirer les noeuds en dehors des bordures
        for (auto &noeud : mapnoeuds) {
            if (!pointEstPossible(noeud.second->point)) {
                if (noeud.second->getTiletype() == TileType::Unknown || noeud.second->getTiletype() == TileType::Forbidden) {
                    noeud.second->setTiletype(TileType::Forbidden);
                } else {
                    throw ExceptionNoeudConnuEnDehors{};
                }
            }
        }
    }
}
