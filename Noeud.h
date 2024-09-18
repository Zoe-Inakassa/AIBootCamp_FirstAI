#ifndef NOEUD_H
#define NOEUD_H

#include <stdexcept>
#include <vector>
#include "Globals.h"

struct Point {
    int q;
    int r;

    Point() = delete;
    Point (const int q, const int r) : q(q), r(r) {}
    
    int calculerDistance(const Point& destination) const;

    bool operator==(const Point& other) const {
        return q == other.q && r == other.r;
    }

    bool operator!=(const Point &other) const {
        return !(*this == other);
    }
    
    static int calculerDistanceCoordonnes(int q1, int r1, int q2, int r2);

    int calculerHash(){ return calculerHash(q, r);}

    static int calculerHash(int q, int r){ return q * 1000000 + r;}
};

enum class TileType{Unknown, Default, Goal, Forbidden};

class Noeud {
public:
    const Point point;
    
private:
    TileType tiletype;
    std::vector<Noeud*> neighbours;

public:
    Noeud() = delete;
    Noeud(const STileInfo& info);
    Noeud(Point point, TileType type);
    ~Noeud() = default;
    
    TileType getTiletype() const
    {
        return tiletype;
    }
    
    void setTiletype(TileType type)
    {
        this->tiletype = type;
    }
    
    const std::vector<Noeud*> &getNeighbours() const
    {
        return neighbours;
    }
    
    void addNeighbour(Noeud* neighbour)
    {
        neighbours.push_back(neighbour);
    }

    void removeNeighbour(const Noeud* neighbour)
    {
        auto p = std::find(neighbours.begin(), neighbours.end(), neighbour);
        if (p == neighbours.end()) throw std::out_of_range("Noeud::removeNeighbour"); // Erreur a possiblement modifier
        Noeud* temp = *p;
        std::swap(*p,neighbours.back());
        neighbours.pop_back();
    }
    
    EHexCellDirection getDirection(const Noeud& other) const;
};

#endif // NOEUD_H
