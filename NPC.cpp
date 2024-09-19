#include "NPC.h"

NPC::NPC(const SNPCInfo &npc, const Noeud *emplacement)
    : id{npc.uid}, state{NPCState::INIT}, emplacement{emplacement}, objectif{nullptr}
{
}

SOrder NPC::deplacer(const Noeud *noeudVoisin)
{
    EHexCellDirection direction = emplacement->getDirection(*noeudVoisin);

    // Déplacement
    emplacement = noeudVoisin;
    
    if(chemin.back()==noeudVoisin) chemin.pop_back();
    
    return SOrder{
        EOrderType::Move,
        id,
        direction
    };
}

int NPC::getId() const
{
    return id;
}

NPCState NPC::getState() const
{
    return state;
}

const Noeud* NPC::getNextTileOnPath()
{
    return chemin.back();
}

void NPC::set_state(NPCState state)
{
    this->state = state;
}
