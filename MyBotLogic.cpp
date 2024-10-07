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

MyBotLogic::MyBotLogic(): maxTurnNumber(0), etatBot{EtatBot::Init}
{
	//Write Code Here
}

MyBotLogic::~MyBotLogic()
{
	for (NPC *pNPC : listeNPC) {
		delete pNPC;
	}
	listeNPC.clear();
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
		auto pNoeud = board.getNoeud(Point{pNPC->q, pNPC->r});
		listeNPC.push_back(new NPC{*pNPC, pNoeud});
	}

	maxTurnNumber = _initData.maxTurnNb;
}

bool MyBotLogic::attribuerObjectifs(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances, bool poidsParMax)
{
	if(mapDistances.empty()) return false;
	//Création d'une solution initiale
	std::map<NPC*,int> solution;
	for(auto npc : mapDistances)
	{
		solution[npc.first] = 0;
	}

	float retour = Solveur::calculerSolution(mapDistances, solution, poidsParMax);
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
		for(NPC *pNPC : listeNPC)
		{
			NPC &npc = *pNPC;
			npc.setState(NPCState::EXPLORATION_PAUSE);
		}
	}
	if(etat == EtatBot::Moving)
	{
		for(NPC *pNPC : listeNPC)
		{
			NPC &npc = *pNPC;
			if(npc.getObjectif()==npc.getEmplacement())
			{
				npc.setState(NPCState::FINISH);
			}else
			{
				std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
				if(!chemin.empty())
				{
					npc.setChemin(chemin);
					npc.setState(NPCState::MOVING);
				}else //Erreur très improbable si les vérifications ont étés faites avant le changement d'état
				{ 
					BOT_LOGIC_LOG(mLogger, "Erreur chemin non trouvé avec Astar", true);
				}
			}
		}
	}
}

void MyBotLogic::calculerScoreExploration(int nbToursRestants)
{
	// Sélectionner les tuiles à explorer selon le score
	// TODO : Éloigner les NPC pour maximiser l'exploration
	mapExplorationDistances.clear();
	for(NPC *pNPC : listeNPC) {
		NPC &npc = *pNPC;
		if (npc.getState() == NPCState::EXPLORATION && npc.getObjectif()->getNbVoisinsUnknown() > 0) {
			// Le NPC se dirige vers une case pouvant être explorée, pas besoin de recalculer
			continue;
		}

		const Point &pointNPC = npc.getEmplacement()->point;

		// Obtenir les noeuds attaignables
		std::vector<SNoeudDistance> noeudsAttaignables = Dijkstra::calculerDistances(
			npc.getEmplacement(),
			[](const Noeud *noeud) { return noeud->getNbVoisinsUnknown() > 0; },
			nbToursRestants);

		// Modifier la valeur pour prendre en compte le score du noeud
		for (auto& noeud : noeudsAttaignables) {
			float distanceNPC = noeud.score;
			float scoreExploration = noeud.pnoeud->getScoreExploration(distanceNPC);
			noeud.score = scoreExploration;
		}
		std::sort(noeudsAttaignables.begin(), noeudsAttaignables.end(),
			[](const SNoeudDistance &noeudA, const SNoeudDistance &noeudB) {
				return noeudA.score < noeudB.score;
			});

		// Ajouter l'emplacement actuel du NPC, pour lui permettre de rester sur place en dernier recours
		noeudsAttaignables.push_back({ npc.getEmplacement(), 1000000 });

		mapExplorationDistances[&npc] = noeudsAttaignables;
	}
}

bool MyBotLogic::CalculerCheminsGoals(int nbToursRestants)
{
	std::map<NPC*, std::vector<SNoeudDistance>> mapDistances;
	for(NPC *pNPC : listeNPC)
	{
		NPC &npc = *pNPC;
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
	std::string log = "GetTurnOrders:--------------------------------Tour n°";
	log += std::to_string(_turnData.turnNb-1);
	BOT_LOGIC_LOG(mLogger, log, true);
	
	//Mettre à jour la vision
	board.updateBoard(_turnData, listeNPC);
	
	if(etatBot == EtatBot::Init)
	{
		BOT_LOGIC_LOG(mLogger, "État Init: (tour 0) passage à un état initial en fonction des conditions de départ", true);
		if (CalculerCheminsGoals(nbToursRestants)) {
			//Les objectifs sont attribués correctement
			setEtatBot(EtatBot::Moving);
		} else {
			//Pas assez d'objectifs ou pas de chemin possible
			setEtatBot(EtatBot::Exploration);
		}
	}
	
	std::map<const Noeud*, NPC*> mouvements;
	if(etatBot == EtatBot::Moving)
	{
		BOT_LOGIC_LOG(mLogger, "État Moving", true);
		
		for(const NPC *pNPC : listeNPC)
		{
			const NPC &npc = *pNPC;
			if(npc.getState()!=NPCState::FINISH && !npc.getEmplacement()->isANeighbour(npc.getNextTileOnPath()))
			{
				BOT_LOGIC_LOG(mLogger, "Prochain mouvement impossible: Changement vers l'état Exploration", true);
				setEtatBot(EtatBot::Exploration);
				break;
			}
		}
		
	}

	if (etatBot == EtatBot::Exploration)
	{
		BOT_LOGIC_LOG(mLogger, "État Exploration", true);

		// S'il y a assez de goal
		if (board.getGoals().size() >= listeNPC.size()) {
			if (CalculerCheminsGoals(nbToursRestants)) {
				// Les objectifs sont attribués
				BOT_LOGIC_LOG(mLogger, "Nouveau chemin vers le(s) goal(s) découvert: Changement vers l'état Moving", true);
				setEtatBot(EtatBot::Moving);
			}
			// Sinon, au moins un NPC n'a pas de goal accessible, continuer d'explorer
		}
	}

	if (etatBot == EtatBot::Exploration)
	{
		for(NPC *pNPC : listeNPC)
		{
			NPC &npc = *pNPC;
			if(npc.getState()!=NPCState::EXPLORATION_PAUSE && !npc.getEmplacement()->isANeighbour(npc.getNextTileOnPath()))
			{
				npc.setState(NPCState::EXPLORATION_PAUSE);
				npc.clearChemin();
			}
		}
		
		// Mettre un score sur les noeuds à explorer (avec des voisins unknown)
		calculerScoreExploration(nbToursRestants);
		// Répartir les noeuds pour que 2 NPC n'explorent pas le même noeud
		attribuerObjectifs(mapExplorationDistances, false);

		for(NPC *pNPC : listeNPC)
		{
			NPC &npc = *pNPC;

			// ignorer les NPC en traint d'explorer
			if (npc.getState() == NPCState::EXPLORATION)
				continue;

			// si un NPC ne bouge pas, il peut explorer au mieux
			if (npc.getObjectif() == npc.getEmplacement() && !mapExplorationDistances.at(pNPC).empty()) {
				npc.setObjectif(mapExplorationDistances.at(pNPC).front().pnoeud);
			}

			debugMapExploration(npc); //affichage des scores de décision pour chaque npc

			std::vector<const Noeud*> chemin = AStar::calculerChemin(npc.getEmplacement(),npc.getObjectif());
			if(!chemin.empty())
			{
				npc.setChemin(chemin);
				npc.setState(NPCState::EXPLORATION);
			}
		}
		mouvements = solveurMouvements(NPCState::EXPLORATION);
	}

	if(etatBot==EtatBot::Moving) mouvements = solveurMouvements(NPCState::MOVING);

	for (auto& mouvement : mouvements)
	{
		_orders.push_back(mouvement.second->deplacer(mouvement.first));
	}
}

std::map<const Noeud*, NPC*> MyBotLogic::solveurMouvements(NPCState npcStateFilter)
{
	std::map<const Noeud*, NPC*> mouvements;
	
	for(NPC *pNPC : listeNPC)
	{
		NPC &npc = *pNPC;
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

void MyBotLogic::debugMapExploration(NPC &npc)
{
	std::string log = "Position NPC n°" + std::to_string(npc.getId());
	log += " : q=" + std::to_string(npc.getEmplacement()->point.q);
	log += ", r=" + std::to_string(npc.getEmplacement()->point.r);
	log += "\n";
	if (mapExplorationDistances.count(&npc)) {
		log += "Score d'exploration:\n";
		for (const SNoeudDistance &distance : mapExplorationDistances.at(&npc))
		{
			log += "    q=" + std::to_string(distance.pnoeud->point.q);
			log += ", r=" + std::to_string(distance.pnoeud->point.r);
			log += " => score=" + std::to_string(distance.score);
			if (distance.pnoeud == npc.getObjectif())
				log += " (OBJECTIF)";
			log += "\n";
		}
	}
	BOT_LOGIC_LOG(mLogger, log, false);
}
