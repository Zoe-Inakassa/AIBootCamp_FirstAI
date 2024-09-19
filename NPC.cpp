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

    return SOrder{
        EOrderType::Move,
        id,
        direction
    };
}
