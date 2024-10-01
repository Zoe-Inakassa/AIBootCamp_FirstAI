#include "MyBotLogic.h"

#include <map>
#include <algorithm>
#include <string>

#include "AStar.h"
#include "Globals.h"
#include "ConfigData.h"
#include "Dijkstra.h"
#include "InitData.h"
#include "Noeud.h"
#include "NPC.h"
#include "TurnData.h"
#include "Solveur.h"

MyBotLogic::MyBotLogic(): etatBot{EtatBot::Init}, maxTurnNumber(0)
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

bool MyBotLogic::attribuerObjectifs(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances)
{
	if(mapDistances.empty()) return false;
	//Création d'une solution initiale
	std::map<NPC*,int> solution;
	for(auto npc : mapDistances)
	{
		solution[npc.first] = 0;
	}

	float retour = Solveur::calculerSolution(mapDistances, solution);
	if(retour == -1)
	{
		BOT_LOGIC_LOG(mLogger, "Aucune solution réalisable n'a été trouvée", true);
		return false;
	}
	
	//Attribuer les objectifs
	for(auto npc : solution)
	{
		npc.first->setObjectif(mapDistances.at(npc.first)[npc.second].pnoeud);
	}

	return true;
}

void MyBotLogic::setEtatBot(const EtatBot& etat)
{
	etatBot = etat;
	if(etat == EtatBot::Exploration) // On presume que le bot était à l'état Init
	{
		for(NPC& npc : listeNPC)
		{
			npc.setState(NPCState::EXPLORATION_PAUSE);
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

void MyBotLogic::calculerScoreExploration(int nbToursRestants)
{
	// Sélectionner les tuiles à explorer selon le score
	// TODO : ne pas oublier d'éloigner les NPC pour maximiser l'exploration
	mapExplorationDistances.clear();
	for (NPC &npc : listeNPC) {
		if (npc.getState() == NPCState::EXPLORATION && npc.getObjectif()->getNbVoisinsUnknown() > 0) {
			// Le NPC se dirige vers une case pouvant être explorée, pas besoin de recalculer
			continue;
		}

		const Point &pointNPC = npc.getEmplacement()->point;

		// Obtenir les noeuds attaignables
		std::vector<SNoeudDistance> noeudsAttaignables = Dijkstra::calculerDistances(
			npc.getEmplacement(),
			[](const Noeud *noeud) { return noeud->getNbVoisinsUnknown() > 0; });

		// Modifier la valeur pour prendre en compte le score du noeud
		for (auto& noeud : noeudsAttaignables) {
			int distance = pointNPC.calculerDistance(noeud.pnoeud->point);
			float scoreExploration = noeud.pnoeud->getScoreExploration(distance);
			noeud.score = scoreExploration;
		}
		std::sort(noeudsAttaignables.begin(), noeudsAttaignables.end(),
			[](const SNoeudDistance &noeudA, const SNoeudDistance &noeudB) {
				return noeudA.score < noeudB.score;
			});

		// Ajouter l'emplacement actuel du NPC, pour lui permettre de rester sur place en dernier recours
		noeudsAttaignables.push_back({ npc.getEmplacement(), INFINITY });

		mapExplorationDistances[&npc] = noeudsAttaignables;
	}
}

bool MyBotLogic::CalculerCheminsGoals(int nbToursRestants)
{
	std::map<NPC*, std::vector<SNoeudDistance>> mapDistances;
	for(NPC& npc : listeNPC)
	{
		// Appliquer dijkstra et vérifier si au moins un objectif est trouvable dans le nombre de tours impartis
		std::vector<SNoeudDistance> distances = Dijkstra::calculerDistances(npc.getEmplacement(), nbToursRestants);
		if(!distances.empty())
		{
			mapDistances[&npc] = distances;
		}else
		{
			BOT_LOGIC_LOG(mLogger, "Un tableau de distances est vide", true);
			return false;
		}
	}
	
	// Attribuer les objectifs
	return attribuerObjectifs(mapDistances);
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	int nbToursRestants = maxTurnNumber - _turnData.turnNb+1;
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);
	
	//Mettre à jour la vision
	board.updateBoard(_turnData, listeNPC);
	
	if(etatBot == EtatBot::Init)
	{
		BOT_LOGIC_LOG(mLogger, "1ère boucle: état Init", true);
		if (CalculerCheminsGoals(nbToursRestants)) {
			// Les objectifs sont attribués
			setEtatBot(EtatBot::Moving);
		} else {
			setEtatBot(EtatBot::Exploration);
		}
	}
	
	if (etatBot == EtatBot::Exploration)
	{
		BOT_LOGIC_LOG(mLogger, "2ème boucle: état Exploration", true);

		// S'il y a assez de goal
		if (board.getGoals().size() >= listeNPC.size()) {
			if (CalculerCheminsGoals(nbToursRestants)) {
				// Les objectifs sont attribués
				setEtatBot(EtatBot::Moving);
			}
			// Sinon, un NPC n'a pas de goal accessible, continuer d'explorer
		}
	}

	std::map<const Noeud*, NPC*> mouvements;
	if(etatBot == EtatBot::Moving)
	{
		BOT_LOGIC_LOG(mLogger, "3ème boucle: état Moving", true);
		
		for(auto npc : listeNPC)
		{
			if(! npc.getEmplacement()->isANeighbour(npc.getNextTileOnPath()))
			{
				setEtatBot(EtatBot::Exploration);
				break;
			}
		}
		
		if(etatBot==EtatBot::Moving) mouvements = solveurMouvements(NPCState::MOVING);
	}

	if (etatBot == EtatBot::Exploration)
	{
		BOT_LOGIC_LOG(mLogger, "4ème boucle: état Exploration", true);
		
		// TODO :Décider du prochain mouvement des npcs
		calculerScoreExploration(nbToursRestants); // à voir
		attribuerObjectifs(mapExplorationDistances);

		BOT_LOGIC_LOG(mLogger, "Score exploration :", true);
		for(NPC& npc : listeNPC)
		{
			std::string log = "  NPC ";
			log += std::to_string(npc.getId());
			log += " : q=";
			log += std::to_string(npc.getEmplacement()->point.q);
			log += ", r=";
			log += std::to_string(npc.getEmplacement()->point.r);
			log += "\n";
			if (mapExplorationDistances.count(&npc)) {
				for (const SNoeudDistance &distance : mapExplorationDistances.at(&npc))
				{
					log += "    q=";
					log += std::to_string(distance.pnoeud->point.q);
					log += ", r=";
					log += std::to_string(distance.pnoeud->point.r);
					log += " => score=";
					log += std::to_string(distance.score);
					if (distance.pnoeud == npc.getObjectif())
						log += " (OBJECTIF)";
					log += "\n";
				}
			}
			BOT_LOGIC_LOG(mLogger, log, false);

			std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
			if(!chemin.empty())
			{
				npc.setChemin(chemin);
				npc.setState(NPCState::EXPLORATION);
			}
		}
		mouvements = solveurMouvements(NPCState::EXPLORATION);
	}

	BOT_LOGIC_LOG(mLogger, "Deplacer les NPC", true);
	for (auto& mouvement : mouvements)
	{
		_orders.push_back(mouvement.second->deplacer(mouvement.first));
	}
}

std::map<const Noeud*, NPC*> MyBotLogic::solveurMouvements(NPCState npcStateFilter)
{
	std::map<const Noeud*, NPC*> mouvements;
	
	for(NPC& npc : listeNPC)
	{
		const Noeud *noeudSuivant = nullptr;
		if(npc.getState() == npcStateFilter)
			noeudSuivant = npc.getNextTileOnPath();

		if(noeudSuivant != nullptr)
		{
			// Vérifier s'il y a conflit et ajouter le mouvement dans mouvements
			if(mouvements.count(noeudSuivant))
			{
				auto npcSurLeNoeud = mouvements[noeudSuivant];
				if(npcSurLeNoeud->tailleChemin() >= npc.tailleChemin())
				{
					// npc ne peut pas bouger
					// npcSurLeNoeud reste sur la tuile
					if(mouvements.count(npc.getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
					mouvements[npc.getEmplacement()] = &npc;
				}
				else
				{
					// npc se déplace sur la tuile
					// npcDejaSurlaTile reste à sa position
					if(mouvements.count(npcSurLeNoeud->getEmplacement())) BOT_LOGIC_LOG(mLogger, "Erreur: NPC ne peut pas avancer ni rester sur la case", true);
					mouvements[npcSurLeNoeud->getEmplacement()] = npcSurLeNoeud;
					mouvements[noeudSuivant] = &npc;
				}
			}
			else
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

	return mouvements;
}
