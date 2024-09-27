#ifndef BOARD_H
#define BOARD_H
#include <map>

#include "InitData.h"
#include "TurnData.h"
#include "Noeud.h"
#include "NPC.h"

class ExceptionNoeudConnuEnDehors{};

class Board{
    std::map<int, Noeud*> mapnoeuds;
    std::set<Noeud*> goals;
    std::map<int, Mur> mapobjets;
    bool goalDecouvert;
    int nombreTileMaxDroite;
    int nombreTileMaxBas;
    
public:
    Board();
    ~Board();
    void initBoard(const SInitData &_initData);
    void updateBoard(const STurnData &_turnData, const std::vector<NPC> &listeNPC);
    void addMur(const SObjectInfo& objet);

    bool existMur(int id)
    {
        return mapobjets.find(id) != mapobjets.end();
    };
    
    void addTile(const STileInfo& tuile);
    
    bool existNoeud(int id)
    {
        return mapnoeuds.find(id) != mapnoeuds.end();
    }

    bool pointEstPossible(Point point) const;
    
    const std::map<int, Noeud*> &getNoeuds() { return mapnoeuds; }

    Noeud *getNoeud(int id)
    {
        return mapnoeuds.at(id);
    }
    
    const std::set<Noeud*> &getGoals()
    {
        return goals;
    }

private:
    void calculerDistancesGoalsTousNoeuds();
    void calculerDistancesGoalsUnNoeud(Noeud *noeud);
    void calculerBordures(const std::vector<NPC> &listeNPC);
};

#endif // BOARD_H
