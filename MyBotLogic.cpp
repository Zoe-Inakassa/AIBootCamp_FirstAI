#include "MyBotLogic.h"

#include <map>
#include <algorithm>

#include "AStar.h"
#include "Globals.h"
#include "ConfigData.h"
#include "Dijkstra.h"
#include "InitData.h"
#include "Noeud.h"
#include "NPC.h"
#include "TurnData.h"

MyBotLogic::MyBotLogic(): etatBot{EtatBot::Init}
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
		const std::vector<SNoeudDistance> &distances = mapDistances.at(&pnpc);
		if (!distances.empty())
			pnpc.setObjectif(distances.front().pnoeud);
	}
}

void MyBotLogic::setEtatBot(const EtatBot& etat)
{
	etatBot = etat;
	if(etat == EtatBot::Exploration) // On presume que le bot était à l'état Init
	{
		for(NPC& npc : listeNPC)
		{
			npc.setState(NPCState::EXPLORATION);
		}
	}
	if(etat == EtatBot::Moving)
	{
		for(NPC& npc : listeNPC)
		{
			std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
			if(!chemin.empty())
			{
				npc.setChemin(chemin);
				npc.setState(NPCState::MOVING);
			}else
			{
				//ERREUR
				BOT_LOGIC_LOG(mLogger, "Erreur chemin non trouvé avec Astar", true);
			}
		}
	}
}

void MyBotLogic::calculerScoreExploration()
{
	scoresExploration.clear();
	int nbToursRestants = INT_MAX; // idée?

	for (auto& noeud : board.getNoeuds()) {
		// S'il y a un intérêt à explorer
		if (noeud.second.getNbVoisinsUnknown() > 0 && noeud.second.getTiletype() != TileType::Unknown) {
			// Trouver la distance à vol d'oiseau avec le NPC le plus proche
			int distanceNPCPlusProche = INT_MAX;
			for (const NPC &npc : listeNPC) {
				int distance = npc.getEmplacement()->point.calculerDistance(noeud.second.point);
				if (distance < distanceNPCPlusProche) {
					distanceNPCPlusProche = distance;
				}
			}
			int distanceMinimum = distanceNPCPlusProche + noeud.second.getDistanceVolGoal();
			if (distanceMinimum <= nbToursRestants) {
				scoresExploration.push_back({
					&noeud.second,
					distanceNPCPlusProche,
					noeud.second.getScoreExploration(distanceNPCPlusProche),
				});
				// Problème pouvant arriver : un npc est proche de 2 tuiles à explorer,
				// Sans considérer que l'autre NPC est très loin
			}
		}
	}

	// Sélectionner les tuiles à explorer selon le score
	// TODO : ne pas oublier d'éloigner les NPC pour maximiser l'exploration
	mapExplorationDistances.clear();
	for (NPC &npc : listeNPC) {
		std::vector<SNoeudDistance> distances;
		const Point &pointNPC = npc.getEmplacement()->point;
		for (auto& noeud : scoresExploration) {
			int distance = pointNPC.calculerDistance(noeud.noeud->point);
			float scoreExploration = noeud.noeud->getScoreExploration(distance);
			int scoreExplorationEntier = 1000 * scoreExploration; // TODO pas bien
			distances.push_back({ noeud.noeud, scoreExplorationEntier });
		}
		mapExplorationDistances[&npc] = distances;
	}
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);
	if(etatBot == EtatBot::Init)
	{
		BOT_LOGIC_LOG(mLogger, "1ère boucle: état Init", true);
		
		std::map<NPC*, std::vector<SNoeudDistance>> mapDistances;
		bool erreur = false;
		for(NPC& npc : listeNPC)
		{
			// Appliquer dijkstra et vérifier si au moins un objectif est trouvable dans le nombre de tours impartis
			std::vector<SNoeudDistance> distances = Dijkstra::calculerDistances(npc.getEmplacement(), maxTurnNumber - _turnData.turnNb+1);
			if(!distances.empty())
			{
				mapDistances[&npc] = distances;
			}else
			{
				erreur = true;
				BOT_LOGIC_LOG(mLogger, "Un tableau de distances est vide", true);
			}
		}
		
		// Attribuer les objectifs
		if(!erreur)
		{
			attribuerObjectifs(mapDistances);
			setEtatBot(EtatBot::Moving);
		}else { setEtatBot(EtatBot::Exploration);
		}
	}
	

	std::map<const Noeud*, NPC*> mouvements;
	if(etatBot == EtatBot::Moving)
	{
		BOT_LOGIC_LOG(mLogger, "2ème boucle: état Moving", true);
		
		for(NPC& npc : listeNPC)
		{
			if(npc.getState() == NPCState::MOVING)
			{
				// Vérifier s'il y a conflit et ajouter le mouvement dans mouvements
				auto noeudSuivant = npc.getNextTileOnPath();
				if(mouvements.count(noeudSuivant))
				{
					auto npcSurLeNoeud = mouvements[noeudSuivant];
					if(npcSurLeNoeud->tailleChemin() >= npc.tailleChemin())
					{
						// npc ne peut pas bouger
						// npcSurLeNoeud reste sur la tuile
						mouvements[npc.getEmplacement()] = &npc;
						if(mouvements.count(npc.getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
					}else
					{
						// npc se déplace sur la tuile
						// npcDejaSurlaTile reste à sa position
						mouvements[npcSurLeNoeud->getEmplacement()] = npcSurLeNoeud;
						if(mouvements.count(npcSurLeNoeud->getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
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
				// il a en théorie toujours la priorité car aucun npc n'est censé passer sur sa tuile
				// Ajouter son "mouvement" dans map pour qu'aucun pion ne vienne sur sa case alors qu'il y est déjà
				if(mouvements.count(npc.getEmplacement()))
				{
					BOT_LOGIC_LOG(mLogger, "Erreur: Quelqu'un veut aller sur la case d'un npc ayant terminé!!!", true);
				}
				mouvements[npc.getEmplacement()] = &npc;
			}
		}
	}

	
	if(etatBot == EtatBot::Exploration)
	{
		BOT_LOGIC_LOG(mLogger, "3ème boucle: état Exploration", true);
		
		//Mettre à jour la vision
		board.updateBoard(_turnData);
		
		// TODO :Décider du prochain mouvement des npcs
		calculerScoreExploration(); // à voir
		attribuerObjectifs(mapExplorationDistances);

		for (NPC& npc : listeNPC)
		{
			std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
			if(!chemin.empty())
			{
				npc.setChemin(chemin);
				npc.setState(NPCState::EXPLORATION);
			}
			if (npc.getState() == NPCState::EXPLORATION)
			{
				// Vérifier s'il y a conflit et ajouter le mouvement dans mouvements
				auto noeudSuivant = npc.getNextTileOnPath();
				if (noeudSuivant != nullptr && mouvements.count(noeudSuivant))
				{
					auto npcSurLeNoeud = mouvements[noeudSuivant];
					if (npcSurLeNoeud->tailleChemin() >= npc.tailleChemin())
					{
						// npc ne peut pas bouger
						// npcSurLeNoeud reste sur la tuile
						mouvements[npc.getEmplacement()] = &npc;
						if (mouvements.count(npc.getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
					}
					else
					{
						// npc se déplace sur la tuile
						// npcDejaSurlaTile reste à sa position
						mouvements[npcSurLeNoeud->getEmplacement()] = npcSurLeNoeud;
						if (mouvements.count(npcSurLeNoeud->getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
						mouvements[noeudSuivant] = &npc;
					}
				}
				else if (noeudSuivant != nullptr)
				{
					// Le noeud est libre, le NPC se déplace
					mouvements[noeudSuivant] = &npc;
				}
			}
			else
			{
				// Le pion reste sur place car il a finit
				// il a en théorie toujours la priorité car aucun npc n'est censé passer sur sa tuile
				// Ajouter son "mouvement" dans map pour qu'aucun pion ne vienne sur sa case alors qu'il y est déjà
				if (mouvements.count(npc.getEmplacement()))
				{
					BOT_LOGIC_LOG(mLogger, "Erreur: Quelqu'un veut aller sur la case d'un npc ayant terminé!!!", true);
				}
				mouvements[npc.getEmplacement()] = &npc;
			}
		}
	}

	BOT_LOGIC_LOG(mLogger, "Deplacer les NPC", true);
	for (auto& mouvement : mouvements)
	{
		_orders.push_back(mouvement.second->deplacer(mouvement.first));
	}
}
