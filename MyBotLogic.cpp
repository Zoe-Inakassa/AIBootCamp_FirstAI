#include "MyBotLogic.h"

#include <map>

#include "AStar.h"
#include "Globals.h"
#include "ConfigData.h"
#include "Dijkstra.h"
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

	maxTurnNumber = _initData.maxTurnNb;
}

void MyBotLogic::attribuerObjectifs(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances)
{
	//Attribuer les objectifs
	if(listeNPC.size() > 1)
	{
		NPC& pnpc1= *listeNPC.begin();
		NPC& pnpc2= *(listeNPC.begin()+1);
		if(mapDistances.at(&pnpc1)[0].pnoeud == mapDistances.at(&pnpc2)[0].pnoeud)
		{
			if(mapDistances.at(&pnpc1)[1].pnoeud > mapDistances.at(&pnpc2)[1].pnoeud)
			{
				pnpc1.setObjectif(mapDistances.at(&pnpc1)[0].pnoeud);
				pnpc2.setObjectif(mapDistances.at(&pnpc2)[1].pnoeud);
			}else
			{
				pnpc1.setObjectif(mapDistances.at(&pnpc1)[1].pnoeud);
				pnpc2.setObjectif(mapDistances.at(&pnpc2)[0].pnoeud);
			}
		}
		else
		{
			pnpc1.setObjectif(mapDistances.at(&pnpc1)[0].pnoeud);
			pnpc2.setObjectif(mapDistances.at(&pnpc2)[0].pnoeud);
		}
	}else
	{
		NPC& pnpc= *listeNPC.begin();
		pnpc.setObjectif(mapDistances.at(&pnpc)[0].pnoeud);
	}
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);

	BOT_LOGIC_LOG(mLogger, "1ère boucle", true);
	std::map<NPC*, std::vector<SNoeudDistance>> mapDistances;
	bool init = false;
	for(NPC& npc : listeNPC)
	{
		if(npc.getState() == NPCState::INIT)
		{
			// Appliquer dijkstra et passer a l'état IDLE
			std::vector<SNoeudDistance> distances = Dijkstra::calculerDistances(npc.getEmplacement(), maxTurnNumber - _turnData.turnNb+1);
			if(!distances.empty())
			{
				mapDistances[&npc] = distances;
				npc.setState(NPCState::IDLE);
				init = true;
			}else
			{
				BOT_LOGIC_LOG(mLogger, "Un tableau de distances est vide", true);
			}
		}
	}

	// Attribuer les objectifs
	if(init) attribuerObjectifs(mapDistances);
	

	BOT_LOGIC_LOG(mLogger, "2ème boucle", true);
	std::map<const Noeud*, NPC*> mouvements;
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
			auto noeudSuivant = npc.getNextTileOnPath();
			if(mouvements.count(noeudSuivant))
			{
				auto npcSurLeNoeud = mouvements[noeudSuivant];
				if(npcSurLeNoeud->tailleChemin() > npc.tailleChemin())
				{
					// npc ne peut pas bouger
					// npcDejaSurlaTile reste sur la tile
					mouvements[npc.getEmplacement()] = &npc;
				}else
				{
					// npc se déplace sur la tile
					// npcDejaSurlaTile reste à sa position
					mouvements[npcSurLeNoeud->getEmplacement()] = npcSurLeNoeud;
					mouvements[noeudSuivant] = &npc;
				}
			}else
			{
				// Le noeud est libre, le NPC se déplace
				mouvements[noeudSuivant] = &npc;
			}
		}
		else
		{
			// Le pion reste sur place car il a finit
			// il a en théorie toujours la priorité mais il pourrait bloquer le chemin donc faire attention
			// Ajouter son "mouvement" dans map pour qu'aucun pion ne vienne sur sa case alors qu'il y est déjà
			if(mouvements.count(npc.getEmplacement()))
			{
				BOT_LOGIC_LOG(mLogger, "Quelqu'un veut aller sur la case d'un npc ayant terminé!!!", true);
			}
			mouvements[npc.getEmplacement()] = &npc;
		}
	}

	BOT_LOGIC_LOG(mLogger, "3ème boucle", true);
	for (auto& mouvement : mouvements)
	{
		_orders.push_back(mouvement.second->deplacer(mouvement.first));
	}
}