#include "MyBotLogic.h"

#include <map>

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

void MyBotLogic::Init(const SInitData& _initData)
{
	BOT_LOGIC_LOG(mLogger, "Init", true);
	
	//Write Code Here
}

void MyBotLogic::GetTurnOrders(const STurnData& _turnData, std::list<SOrder>& _orders)
{
	BOT_LOGIC_LOG(mLogger, "GetTurnOrders", true);

	BOT_LOGIC_LOG(mLogger, "1ère boucle", true);
	for(int i = 0;i < _turnData.npcInfoArraySize; i++)
	{
		
	}

	BOT_LOGIC_LOG(mLogger, "2ème boucle", true);
	std::map<Noeud, NPC> mouvements;
	for(int i = 0;i < _turnData.npcInfoArraySize; i++)
	{
		
	}

	BOT_LOGIC_LOG(mLogger, "3ème boucle", true);
	for (auto& mouvement : mouvements)
	{
		mouvement.second.deplacer(&mouvement.first);
	}
}