#ifndef NOEUD_H
#define NOEUD_H

#include <stdexcept>
#include <vector>
#include <set>
#include "Globals.h"
#include "Mur.h"

class ExceptionCellTypeInconnu{};
class ExceptionChangementTileTypeInvalide{};

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

    // Comparaison utilisée dans mapnoeuds
    bool operator<(const Point &point)  const {
        if (q != point.q) return q < point.q;
        return r < point.r;
    }
    
    static int calculerDistanceCoordonnes(int q1, int r1, int q2, int r2);

    std::vector<Point> surroundingPoints() const;
};

enum class TileType{Unknown, Default, Goal, Forbidden};

class Noeud {
public:
    const Point point;
    
private:
    TileType tiletype;
    std::set<Noeud*> neighbours;
    std::set<const Mur*> murs;
    int nbVoisinsUnknown;
    int heuristiqueDistanceVolGoal = 0;

public:
    // On delete le constructeur par défaut et de recopie pour éviter les erreurs
    Noeud() = delete;
    Noeud(const Noeud& other) = delete;
    Noeud &operator=(const Noeud& other) = delete;
    
    Noeud(const STileInfo& info);
    Noeud(Point point, TileType type);
    ~Noeud() = default;
    
    TileType getTiletype() const
    {
        return tiletype;
    }
    
    void setTiletype(TileType type);
    
    const std::set<Noeud*> &getNeighbours() const
    {
        return neighbours;
    }
    
    void addNeighbour(Noeud* neighbour);
    void removeNeighbour(const Noeud* neighbour);
    void addMur(const Mur *mur);
    bool hasOpaqueMur(EHexCellDirection cellDirection) const;
    EHexCellDirection getDirection(const Noeud& other) const;
    Point getPointNeighbour(EHexCellDirection cellDirection) const;
    int getNbVoisinsUnknown() const { return nbVoisinsUnknown; }
    int getDistanceVolGoal() const { return heuristiqueDistanceVolGoal; }
    void setDistanceVolGoal(int distance) { heuristiqueDistanceVolGoal = distance; }
    float getScoreExploration(int distanceNPC) const;

    bool isANeighbour(const Noeud* neighbour) const;
};

#endif // NOEUD_H
