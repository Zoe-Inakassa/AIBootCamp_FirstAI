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

const Noeud* NPC::getNextTileOnPath() const
{
    return chemin.back();
}

void NPC::setChemin(std::vector<const Noeud*>& chemin)
{
    this->chemin=chemin;
}

void NPC::setObjectif(const Noeud* objectif)
{
    this->objectif = objectif;
}

void NPC::setState(NPCState state)
{
    this->state = state;
}
