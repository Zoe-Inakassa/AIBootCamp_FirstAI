#pragma once

#include "BotLogicIF.h"
#include "Logger.h"
#include "Board.h"
#include "NPC.h"

#ifdef _DEBUG
#define BOT_LOGIC_DEBUG
#endif

#ifdef BOT_LOGIC_DEBUG
#define BOT_LOGIC_LOG(logger, text, autoEndLine) logger.Log(text, autoEndLine)
#else
#define BOT_LOGIC_LOG(logger, text, autoEndLine) 0
#endif

struct SNoeudDistance;
struct SConfigData;
struct STurnData;

enum class EtatBot {Init, Moving, Exploration};

struct SScoreExploration {
	const Noeud *noeud;
	int distanceVolNPC;
	float scoreExploration;
};

//Custom BotLogic where the AIBot decision making algorithms should be implemented.
//This class must be instantiated in main.cpp.
class MyBotLogic : public virtual BotLogicIF
{
public:
	MyBotLogic();
	virtual ~MyBotLogic();

	virtual void Configure(const SConfigData& _configData);
	virtual void Init(const SInitData& _initData);
	virtual bool CalculerCheminsGoals(int nbToursRestants);
	virtual void GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders);
	bool attribuerObjectifs(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances);
	void setEtatBot(const EtatBot&);
	void calculerScoreExploration(int nbToursRestants);
	std::map<const Noeud*, NPC*> solveurMouvements(NPCState npcStateFilter);

protected:
	Logger mLogger;
	Board board;
	std::vector<NPC> listeNPC;
	std::map<NPC*, std::vector<SNoeudDistance>> mapExplorationDistances;
	int maxTurnNumber;
	EtatBot etatBot;

};