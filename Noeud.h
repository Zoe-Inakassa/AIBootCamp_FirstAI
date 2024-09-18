#ifndef NOEUD_H
#define NOEUD_H

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

    int calculerHash(){ return q * 1000000 + r;}
};

enum class TyleType{Unknown, Default, Goal, Forbidden};

class Noeud {
public:
    const Point point;
    
private:
    TyleType type;
    std::vector<Noeud*> neighbours;

public:
    Noeud() = delete;
    Noeud(Point point, TyleType type);
    ~Noeud() = default;
    
    TyleType type1() const
    {
        return type;
    }
    
    void set_type(TyleType type)
    {
        this->type = type;
    }
    
    const std::vector<Noeud*> &neighbours1() const
    {
        return neighbours;
    }
    
    void set_neighbours(const std::vector<Noeud*>& neighbours)
    {
        this->neighbours = neighbours;
    }
    
    EHexCellDirection getDirection(const Noeud& other) const;
};

#endif // NOEUD_H
