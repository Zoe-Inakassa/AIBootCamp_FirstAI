#ifndef BOARD_H
#define BOARD_H
#include <map>

#include "InitData.h"
#include "Noeud.h"

class Board{
    std::map<int, Noeud> mapnoeuds;
    std::vector<Noeud*> goals;
    
public:
    Board();
    void initBoard(const SInitData &_initData);

    Noeud *getNoeud(int id)
    {
        return &mapnoeuds[id];
    }
    
    const std::vector<Noeud*> &getGoals()
    {
        return goals;
    }
};

#endif // BOARD_H
