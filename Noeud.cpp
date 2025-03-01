﻿#include "Noeud.h"


int Point::calculerDistance(const Point &destination) const {
    return calculerDistanceCoordonnes(q, r, destination.q, destination.r);
}

int Point::calculerDistanceCoordonnes(int q1, int r1, int q2, int r2) {
    return (abs(q1 - q2) 
          + abs(q1 + r1 - q2 - r2)
          + abs(r1 - r2)) / 2;
}

std::vector<Point> Point::surroundingPoints() const
{
    return { Point(q-1, r), Point(q-1, r+1),
        Point(q, r+1), Point(q+1, r),
        Point(q+1, r-1), Point(q, r-1)};
}

Noeud::Noeud(const STileInfo &info): point(info.q, info.r), nbVoisinsUnknown{0}
{
    switch (info.type) {
        case EHexCellType::Default:
            tiletype=TileType::Default;
            break;
        case EHexCellType::Forbidden:
            tiletype=TileType::Forbidden;
            break;
        case EHexCellType::Goal:
            tiletype=TileType::Goal;
            break;
        default:
            throw ExceptionCellTypeInconnu{};
    }
}

Noeud::Noeud(Point point, TileType type): point{point}, tiletype{type}, nbVoisinsUnknown{0}
{
    
}

void Noeud::setTiletype(TileType type)
{
    if (this->tiletype == type)
        return;
    if (this->tiletype != TileType::Unknown)
        throw ExceptionChangementTileTypeInvalide{};
    // Mettre à jour les voisins
    switch (type) {
        case TileType::Default:
        case TileType::Goal:
            // Les voisins ont 1 unknown de moins
            for (auto voisin : neighbours) {
                voisin->nbVoisinsUnknown--; // Retirer du compteur de unknown
            }
            break;
        case TileType::Forbidden:
            // Les voisins ont 1 voisin (et 1 unknown) de moins
            for (auto voisin : neighbours) {
                voisin->removeNeighbour(this);
            }
            // Forbidden n'a plus de voisins
            neighbours.clear();
            break;
        default:
            // Unknown ne peut pas arriver
            throw ExceptionChangementTileTypeInvalide{};
    }
    this->tiletype = type;
}

void Noeud::addNeighbour(Noeud* neighbour)
{
    if (neighbours.count(neighbour))
        return;
    if (neighbour->getTiletype() == TileType::Forbidden)
        return;
    for (auto mur : murs) {
        // Vérifier les murs
        if (mur->getNoeudOppose(this) == neighbour)
            return;
    }
    neighbours.insert(neighbour);
    if (neighbour->getTiletype() == TileType::Unknown)
        nbVoisinsUnknown++; // Ajouter dans le compteur de unknown
}

void Noeud::removeNeighbour(const Noeud* neighbour)
{
    auto it = neighbours.find(const_cast<Noeud*>(neighbour));
    if (it != neighbours.end()) {
        neighbours.erase(it);
        if (neighbour->getTiletype() == TileType::Unknown)
            nbVoisinsUnknown--; // Retirer du compteur de unknown
    }
}

void Noeud::addMur(const Mur* mur)
{
    murs.insert(mur);
    const Noeud *oppose = mur->getNoeudOppose(this);
    removeNeighbour(oppose);
}

bool Noeud::hasOpaqueMur(EHexCellDirection cellDirection) const
{
    for (auto mur : murs) {
        if (!mur->transparent && getDirection(*mur->getNoeudOppose(this)) == cellDirection) {
            return true;
        }
    }
    return false;
}

EHexCellDirection Noeud::getDirection(const Noeud& other) const
{
    int deltaQ = other.point.q - point.q;
    int deltaR = other.point.r - point.r;
    if (deltaQ + deltaR == 0) {
        // Diagonale NE ou SW
        if (deltaQ == 0) return CENTER;
        return deltaQ > 0 ? SW : NE;
    }
    else if (deltaQ + deltaR < 0) {
        // W et NW
        return deltaQ == 0 ? W : NW;
    }
    else {
        // E et SE
        return deltaQ == 0 ? E : SE;
    }
}

Point Noeud::getPointNeighbour(EHexCellDirection cellDirection) const
{
    switch (cellDirection) {
        case W: return Point{point.q, point.r - 1};
        case NW: return Point{point.q - 1, point.r};
        case NE: return Point{point.q - 1, point.r + 1};
        case E: return Point{point.q, point.r + 1};
        case SE: return Point{point.q + 1, point.r};
        case SW: return Point{point.q + 1, point.r - 1};
        case CENTER: return point;
    }
    throw std::exception("Unreachable");
}

float Noeud::getScoreExploration(float distanceNPC) const
{
    // un noeud a maximum 5 voisins unknown
    return COEFFICIENT_VOISINS_UNKNOWN * static_cast<float>(5-nbVoisinsUnknown)
        + static_cast<float>(distanceNPC) * COEFFICIENT_DISTANCE_NPC
        + static_cast<float>(heuristiqueDistanceVolGoal) * COEFFICIENT_DISTANCE_GOAL;
}

bool Noeud::isANeighbour(const Noeud* neighbour) const
{
    for(const Noeud* noeud: neighbours)
    {
        if(noeud == neighbour) return true;
    }
    return false;
}
