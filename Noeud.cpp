#include "Noeud.h"


int Point::calculerDistance(const Point &destination) const {
    return (abs(destination.q - q) 
          + abs(destination.q + destination.r - q - r)
          + abs(destination.r - r)) / 2;
}

int Point::calculerDistanceCoordonnes(int q1, int r1, int q2, int r2) {
    Point p1 = Point(q1, r1);
    Point p2 = Point(q2, r2);
    return p1.calculerDistance(p2);
}

Noeud::Noeud(Point point, TyleType type): point{point}, type{type}
{
    
}

EHexCellDirection Noeud::getDirection(Noeud& other) const
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
