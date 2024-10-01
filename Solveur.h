#pragma once

#include "NPC.h"
#include "Dijkstra.h"
#include <map>

class Solveur {
public:
    static std::pair<NPC*,NPC*> realisable(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances, const std::map<NPC*,int>& solution);
	static float calculerSolution(const std::map<NPC*, std::vector<SNoeudDistance>>& mapDistances, std::map<NPC*,int>& solution);
};
