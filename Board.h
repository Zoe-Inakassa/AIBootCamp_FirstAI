#ifndef BOARD_H
#define BOARD_H
#include <map>

#include "InitData.h"
#include "Noeud.h"

class Board{
    std::map<int, Noeud> mapnoeuds;
    std::vector<Noeud*> goals;
    std::map<int, Mur> mapobjets;
    
public:
    Board();
    void initBoard(const SInitData &_initData);
    void addMur(SObjectInfo* pObjet);

    bool existNoeud(int id)
    {
        return mapnoeuds.find(id) != mapnoeuds.end();
    }
    
    Noeud *getNoeud(int id)
    {
        return &mapnoeuds.at(id);
    }
    
    const std::vector<Noeud*> &getGoals()
    {
        return goals;
    }
};

#endif // BOARD_H
