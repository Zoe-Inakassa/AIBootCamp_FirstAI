#include "MyBotLogic.h"

#include <map>
#include <string>

#include "AStar.h"
#include "Globals.h"
#include "ConfigData.h"
#include "InitData.h"
#include "Noeud.h"
#include "NPC.h"
#include "TurnData.h"

MyBotLogic::MyBotLogic()
{
	//Write Code Here
}

MyBotLogic::~MyBotLogic()
{
	//Write Code Here
}

void MyBotLogic::Configure(const SConfigData& _configData)
{
#ifdef BOT_LOGIC_DEBUG
	mLogger.Init(_configData.logpath, "MyBotLogic.log");
#endif

	BOT_LOGIC_LOG(mLogger, "Configure", true);

	//Write Code Here
}

void MyBotLogic::debugNoeud(const Noeud *noeud) {
	std::string str;
	str += std::to_string(noeud->point.q);
	str += ",";
	str += std::to_string(noeud->point.r);
	str += ":";
	switch (noeud->getTiletype()) {
		case TileType::Default: str += "default"; break;
		case TileType::Forbidden: str += "forbidden"; break;
		case TileType::Unknown: str += "unknown"; break;
		case TileType::Goal: str += "goal"; break;
	}
	mLogger.Log(str);
}

#include "AStar.h"
#include "Board.h"
void MyBotLogic::Init(const SInitData& _initData)
{
	BOT_LOGIC_LOG(mLogger, "Init", true);
	
	// Enregistrer les tiles
	board.initBoard(_initData);

	// Enregistrer les NPC
	listeNPC.reserve(_initData.nbNPCs);
	for (auto pNPC = _initData.npcInfoArray; pNPC < _initData.npcInfoArray + _initData.nbNPCs; ++pNPC) {
		auto pNoeud = board.getNoeud(Point::calculerHash(pNPC->q, pNPC->r));
		listeNPC.push_back(NPC{*pNPC, pNoeud});
	}
	
	// Execution uniquement avec L_022 !
	// NE PAS MERGE
	mLogger.Log("Parcours des tiles");
	for (int r = 0; r < 4; r++) {
		debugNoeud(board.getNoeud(r));
	}
	debugNoeud(board.getGoals().at(0));
	mLogger.Log("NPC : " + std::to_string(_initData.npcInfoArray[0].q));
	mLogger.Log("NPC : " + std::to_string(_initData.npcInfoArray[0].r));

	mLogger.Log("goal...");
	const Noeud *goal = board.getNoeud(Point::calculerHash(0, 4));
	mLogger.Log("npc...");
	const Noeud *npc = board.getNoeud(Point::calculerHash(0, 0));
	mLogger.Log("Execution de A*");
	auto chemin = AStar::calculerChemin(npc, goal);
	mLogger.Log("Chemin : ");
	for (auto noeud : chemin) {
		debugNoeud(noeud);
	}

}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);

	BOT_LOGIC_LOG(mLogger, "1ère boucle", true);
	for(NPC& npc : listeNPC)
	{
		if(npc.getState() == NPCState::INIT)
		{
			// Appliquer dijkstra et passer a l'état IDLE + modifier l'objectif
			// A CODER
		}
	}

	BOT_LOGIC_LOG(mLogger, "2ème boucle", true);
	std::map<Noeud*, NPC> mouvements;
	for(NPC& npc : listeNPC)
	{
		// Si a l'état IDLE calculer chemin avec astar et passer en marche
		if(npc.getState() == NPCState::IDLE)
		{
			std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
			if(!chemin.empty())
			{
				npc.setChemin(chemin);
				npc.setState(NPCState::MOVING);
			}
		}
		if(npc.getState() == NPCState::MOVING)
		{
			// Vérifier si il y a conflit et ajouter le mouvement dans mouvements
			// A CODER
		}
		else
		{
			// Le pion reste sur place car il a finit
			// il a en théorie toujours la priorité mais il pourrait bloquer le chemin donc faire attention
			// Ajouter son "mouvement" dans map pour qu'aucun pion ne vienne sur sa case alors qu'il y est déjà
			// A CODER
		}
	}

	BOT_LOGIC_LOG(mLogger, "3ème boucle", true);
	for (auto& mouvement : mouvements)
	{
		mouvement.second.deplacer(mouvement.first);
	}
}