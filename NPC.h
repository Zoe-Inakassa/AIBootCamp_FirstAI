#pragma once

// #include "Noeud.h"
class Noeud; // TODO
#include "Globals.h"
#include <vector>

enum NPCState {
    INIT,
    IDLE,
    MOVING,
    FINISH
};

class NPC
{
public:
    NPC(SNPCInfo npc, const Noeud *emplacement);

private:
    const int id;
    NPCState state;
    const Noeud *emplacement; // Tile actuel
    const Noeud *objectif; // Goal ou objectif du chemin
    std::vector<const Noeud*> chemin; // Chemin vers l'objectif
};
