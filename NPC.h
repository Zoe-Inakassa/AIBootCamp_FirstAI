#pragma once

#include "Noeud.h"
#include "Globals.h"
#include <vector>
#include <list>

enum class NPCState {
    INIT,
    EXPLORATION_PAUSE,
    EXPLORATION,
    MOVING,
    FINISH
};

class NPC
{
public:
    NPC(const SNPCInfo &npc, const Noeud *emplacement);
    NPC(const NPC& other) = delete;
    NPC& operator=(const NPC& other) = delete;

    SOrder deplacer(const Noeud *voisin);
    const Noeud *getEmplacement() const { return emplacement; }
    const Noeud *getObjectif() const { return objectif; }
    void setObjectif(const Noeud* objectif);
    int getId() const;
    NPCState getState() const;
    void setState(NPCState state);
    const Noeud* getNextTileOnPath() const;
    void setChemin(std::vector<const Noeud*>& chemin);
    int tailleChemin() const;
    void clearChemin();

private:
    const int id;
    NPCState state;
    const Noeud *emplacement; // Tile actuel
    const Noeud *objectif; // Goal ou objectif du chemin
    std::vector<const Noeud*> chemin; // Chemin vers l'objectif
};
