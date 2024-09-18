#include "NPC.h"

NPC::NPC(SNPCInfo npc, const Noeud *emplacement)
    : id{npc.uid}, state{NPCState::INIT}, emplacement{emplacement}, objectif{nullptr}
{
}

SOrder NPC::deplacer(const Noeud *noeudVoisin)
{
    // TODO
    // EHexCellDirection direction = emplacement->getDirection(noeudVoisin);
    EHexCellDirection direction = EHexCellDirection::CENTER;

    // Déplacement
    emplacement = noeudVoisin;

    return SOrder{
        EOrderType::Move,
        id,
        direction
    };
}
