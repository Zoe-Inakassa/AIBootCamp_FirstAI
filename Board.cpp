#include "Board.h"

Board::Board(): mapnoeuds{}, goals{}, nombreTileMaxDroite{INT_MAX}, nombreTileMaxBas{INT_MAX},
    nombreTileMaxDroitePair{INT_MAX}, nombreTileMaxDroiteImpair{INT_MAX}
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
        if(!existMur(pObjet->uid))
        {
            addMur(*pObjet);
        }
    }

    // Enregistrer les voisins
    for (auto& noeud : mapnoeuds) {
        std::vector<Point> points = noeud.second->point.surroundingPoints();
        for (auto& point : points)
        {
            auto adresse = mapnoeuds.find(point);
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

void Board::updateBoard(const STurnData &_turnData, const std::vector<NPC*> &listeNPC)
{
    goalDecouvert = false;
    for(int i=0; i!=_turnData.tileInfoArraySize; i++)
	{
		STileInfo tuile = _turnData.tileInfoArray[i];
		addTile(tuile);
        Point point{tuile.q, tuile.r};
        calculerDistancesGoalsUnNoeud(mapnoeuds.at(point));
	}
	
	for(int i=0; i!=_turnData.objectInfoArraySize; i++)
	{
		SObjectInfo object = _turnData.objectInfoArray[i];
		if(!existMur(object.uid))
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
    if (!mapobjets.count(objet.uid))
    {
        // Obtenir ou créer le noeud
        Noeud *noeudA = getOrCreateNoeudFictif(pointA);
        
        // Obtenir ou créer le noeud
        Point pointB = noeudA->getPointNeighbour(objet.cellPosition);
        Noeud *noeudB = getOrCreateNoeudFictif(pointB);

        bool transparent = false;
        for (int *type = objet.types; type != objet.types + objet.typesSize; ++type) {
            if (*type == EObjectType::Window)
                transparent = true;
        }

        mapobjets[objet.uid] = {
            noeudA,
            noeudB,
            transparent
        };
        const Mur *mur = &mapobjets.at(objet.uid);

        noeudA->addMur(mur);
        noeudB->addMur(mur);
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

void Board::addTile(const STileInfo &tuile)
{
    Point point = Point{tuile.q, tuile.r};
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
    Noeud *noeud = getOrCreateNoeud(point, tiletype, true);

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

            Noeud *noeudVoisin = getOrCreateNoeud(pointVoisin, TileType::Unknown, false);
            if (noeudVoisin->getTiletype() == TileType::Unknown) {
                noeudVoisin->addNeighbour(noeud);
                noeud->addNeighbour(noeudVoisin);
            }
        }
    }
}

Noeud *Board::getOrCreateNoeud(Point point, TileType type, bool updateType)
{
    auto itNoeud = mapnoeuds.find(point);

    if (!pointEstPossible(point)) {
        type = TileType::Forbidden;
    }

    if (itNoeud != mapnoeuds.end()) {
        Noeud *noeud = itNoeud->second;
        if(updateType && noeud->getTiletype() == TileType::Unknown) {
            noeud->setTiletype(type);
        }
        return noeud;
    }

    return mapnoeuds[point] = new Noeud(point, type);
}

Noeud *Board::getOrCreateNoeudFictif(Point point)
{
    return getOrCreateNoeud(point, TileType::Unknown, false);
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

void Board::calculerBordures(const std::vector<NPC*> &listeNPC)
{
    // Si un NPC pourrait voir une tile mais que celle-ci n'existe pas ou est UNKNOWN
    // Alors, nous sommes à la bordure
    // La bordure gauche et haute sont définies par 0,0
    bool bordureChangee = false;

    // Trouver la bordure Est
    if (nombreTileMaxDroitePair == INT_MAX || nombreTileMaxDroiteImpair == INT_MAX) {
        const Noeud *noeudBordureE = trouverNoeudBordure(listeNPC, EHexCellDirection::E);
        if (noeudBordureE != nullptr) {
            int xBordure = noeudBordureE->point.q + 2 * noeudBordureE->point.r;
            // Selon le niveau, la bordure à droite peut être plus loin sur les q pairs et impairs
            if (noeudBordureE->point.q % 2 == 0) {
                nombreTileMaxDroitePair = xBordure;
            } else {
                nombreTileMaxDroiteImpair = xBordure;
            }
            nombreTileMaxDroite = std::max(
                // en ligne paire, la bordure peut être 1 plus à droite ou 1 à gauche
                nombreTileMaxDroitePair == INT_MAX ? nombreTileMaxDroiteImpair + 1 : nombreTileMaxDroitePair,
                // en ligne impaire, la bordure peut être 1 plus à droite ou 1 à gauche
                nombreTileMaxDroiteImpair == INT_MAX ? nombreTileMaxDroitePair + 1 : nombreTileMaxDroiteImpair);
            bordureChangee = true;
        }

        const Noeud *noeudBordureNE = trouverNoeudBordure(listeNPC, EHexCellDirection::NE);
        if (noeudBordureNE != nullptr) {
            int xBordure = noeudBordureNE->point.q + 2 * noeudBordureNE->point.r;
            // Selon le niveau, la bordure à droite peut être plus loin sur les q pairs et impairs
            if (noeudBordureNE->point.q % 2 == 0) {
                nombreTileMaxDroiteImpair = xBordure - 1;
            } else {
                nombreTileMaxDroitePair = xBordure - 1;
            }
            nombreTileMaxDroite = std::max(
                // en ligne paire, la bordure peut être 1 plus à droite ou 1 à gauche
                nombreTileMaxDroitePair == INT_MAX ? nombreTileMaxDroiteImpair + 1 : nombreTileMaxDroitePair,
                // en ligne impaire, la bordure peut être 1 plus à droite ou 1 à gauche
                nombreTileMaxDroiteImpair == INT_MAX ? nombreTileMaxDroitePair + 1 : nombreTileMaxDroiteImpair);
            bordureChangee = true;
        }
    }

    // Trouver la bordure Sud
    if (nombreTileMaxBas == INT_MAX) {
        const Noeud *noeudBordureSE = trouverNoeudBordure(listeNPC, EHexCellDirection::SE);
        if (noeudBordureSE != nullptr) {
            nombreTileMaxBas = noeudBordureSE->point.q;
            bordureChangee = true;
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

const Noeud *Board::trouverNoeudBordure(const std::vector<NPC*> &listeNPC, EHexCellDirection direction)
{
    for(NPC *pNPC : listeNPC) {
        const Noeud *noeud = pNPC->getEmplacement();

        for (int distance = 0; distance < pNPC->getVisionRange(); ++distance) {
            // Arrêter si un mur bloque la vue
            if (noeud->hasOpaqueMur(direction)) {
                break;
            }

            Point pointSuivant = noeud->getPointNeighbour(direction);
            // Vérifier que le point est possible avec les autres bordures
            if (!pointEstPossible(pointSuivant)) {
                break;
            }

            Noeud *noeudSuivant = existNoeud(pointSuivant) ? getNoeud(pointSuivant) : nullptr;
            // Les noeuds voisins existent forcément, sauf cas de bordure
            if (noeudSuivant == nullptr || noeudSuivant->getTiletype() == TileType::Unknown) {
                if (noeudSuivant != nullptr) {
                    // le noeud est en dehors de la carte
                    noeudSuivant->setTiletype(TileType::Forbidden);
                }
                return noeud;
            }

            // Continuer depuis le nouveau noeud
            noeud = noeudSuivant;
        }
    }

    // Pas de noeud de bordure trouvé
    return nullptr;
}
