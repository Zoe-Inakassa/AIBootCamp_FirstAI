#ifndef BOARD_H
#define BOARD_H
#include <map>

#include "InitData.h"
#include "TurnData.h"
#include "Noeud.h"
#include "NPC.h"

class ExceptionNoeudConnuEnDehors{};

class Board{
    std::map<Point, Noeud*> mapnoeuds;
    std::set<Noeud*> goals;
    std::map<int, Mur> mapobjets;
    bool goalDecouvert;
    int nombreTileMaxDroite;
    int nombreTileMaxBas;
    
public:
    Board();
    ~Board();
    void initBoard(const SInitData &_initData);
    void updateBoard(const STurnData &_turnData, const std::vector<NPC*> &listeNPC);
    void addMur(const SObjectInfo& objet);

    bool existMur(int id) const
    {
        return mapobjets.count(id);
    };
    
    bool existNoeud(Point point) const
    {
        return mapnoeuds.count(point);
    }

    bool pointEstPossible(Point point) const;
    
    const std::map<Point, Noeud*> &getNoeuds() { return mapnoeuds; }

    Noeud *getNoeud(Point point)
    {
        return mapnoeuds.at(point);
    }
    
    const std::set<Noeud*> &getGoals() const
    {
        return goals;
    }

private:
    void addTile(const STileInfo& tuile);
    Noeud *getOrCreateNoeud(Point point, TileType type, bool updateType);
    Noeud *getOrCreateNoeudFictif(Point point);
    
    void calculerDistancesGoalsTousNoeuds();
    void calculerDistancesGoalsUnNoeud(Noeud *noeud);
    void calculerBordures(const std::vector<NPC*> &listeNPC);
};

#endif // BOARD_H
