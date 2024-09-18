#pragma once

#include "Noeud.h"
#include "Globals.h"
#include <vector>
#include <list>

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
    SOrder deplacer(const Noeud *voisin);

private:
    const int id;
    NPCState state;
    const Noeud *emplacement; // Tile actuel
    const Noeud *objectif; // Goal ou objectif du chemin
    std::vector<const Noeud*> chemin; // Chemin vers l'objectif
};
