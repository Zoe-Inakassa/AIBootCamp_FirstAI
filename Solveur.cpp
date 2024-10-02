#include "Solveur.h"

std::pair<NPC *, NPC *> Solveur::realisable(const std::map<NPC *, std::vector<SNoeudDistance>> &mapDistances, const std::map<NPC *, int> &solution)
{
	std::map<const Noeud*, NPC*> noeuds;
	for(auto npc : solution)
	{
		if(mapDistances.count(npc.first))
		{
			const Noeud* noeudATester = mapDistances.at(npc.first)[npc.second].pnoeud;
			auto it = noeuds.find(noeudATester);
			if(it!=noeuds.end())
			{
				return {it->second,npc.first};
			}
			noeuds[noeudATester] = npc.first;
		}
	}
	return {nullptr,nullptr};
}

float Solveur::calculerSolution(
	const std::map<NPC *, std::vector<SNoeudDistance>> &mapDistances,
	std::map<NPC *, int> &solution,
	bool poidsPaxMax)
{
	std::pair<NPC*,NPC*> npcs = realisable(mapDistances, solution);
	if(npcs.first == nullptr) //realisable
	{
		float max = -1;
		for(auto npc : solution)
		{
			if(!mapDistances.at(npc.first).empty())
			{
				float valeur = mapDistances.at(npc.first)[npc.second].score;
				if (poidsPaxMax) {
					if(valeur > max) max = valeur;
				} else {
					max += valeur;
				}
			}
		}
		return max;
	}
	float value1 = -1;
	float value2 = -1;
	
	//Relancer sur le premier conflit
	std::map<NPC*,int> copiesolution1;
	if(mapDistances.at(npcs.first).size() > solution.at(npcs.first)+1)
	{
		copiesolution1 = solution;
		copiesolution1[npcs.first] = copiesolution1.at(npcs.first)+1;
		value1 = calculerSolution(mapDistances, copiesolution1, poidsPaxMax);
	}
	
	//Relancer sur le deuxième conflit
	std::map<NPC*,int> copiesolution2;
	if(mapDistances.at(npcs.second).size() > solution.at(npcs.second)+1)
	{
		copiesolution2 = solution;
		copiesolution2[npcs.second] = copiesolution2.at(npcs.second)+1;
		value2 = calculerSolution(mapDistances, copiesolution2, poidsPaxMax);
	}

	// Retourne la solution avec le score le plus faible et qui fonctionne (qui n'est pas NaN)
	if(value1 == -1)
	{
		solution = copiesolution2;
		return value2;
	}
	if(value2 == -1 || value1 < value2)
	{
		solution = copiesolution1;
		return value1;
	}
	solution = copiesolution2;
	return value2;
}
