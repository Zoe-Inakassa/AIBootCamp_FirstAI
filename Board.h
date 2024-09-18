#ifndef BOARD_H
#define BOARD_H
#include <map>

#include "Noeud.h"

class Board{
    std::map<int, Noeud*> mapnoeuds;
    std::vector<Noeud*> goals;
};

#endif // BOARD_H
