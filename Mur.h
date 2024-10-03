#pragma once

#include "Globals.h"

class Noeud;

struct Mur {
    const Noeud *noeudA;
    const Noeud *noeudB;
    bool transparent = false;

    const Noeud *getNoeudOppose(const Noeud *actuel) const {
        return noeudA == actuel ? noeudB : noeudA;
    }
};
