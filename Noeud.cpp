#include "Noeud.h"


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

Noeud::Noeud(const STileInfo &info): point(info.q, info.r)
{
    if(info.type == EHexCellType::Default) tiletype=TileType::Default;
    if(info.type == EHexCellType::Forbidden) tiletype=TileType::Forbidden;
    if(info.type == EHexCellType::Goal) tiletype=TileType::Goal;
}

Noeud::Noeud(Point point, TileType type): point{point}, tiletype{type}
{
    
}

EHexCellDirection Noeud::getDirection(const Noeud& other) const
{
    int deltaQ = other.point.q - point.q;
    int deltaR = other.point.r - point.r;
    if (deltaQ + deltaR == 0) {
        // Diagonale NE ou SW
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
    }
}
