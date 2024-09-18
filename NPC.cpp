#include "NPC.h"

NPC::NPC(SNPCInfo npc, const Noeud *emplacement)
    : id{npc.uid}, state{NPCState::INIT}, emplacement{emplacement}, objectif{nullptr}
{
}
