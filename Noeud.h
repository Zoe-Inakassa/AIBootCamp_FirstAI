#ifndef NOEUD_H
#define NOEUD_H

#include <stdexcept>
#include <vector>
#include <set>
#include "Globals.h"
#include "Mur.h"

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

    int calculerHash() const { return calculerHash(q, r);}

    // 4 bits pour la direction du Mur | q limite [-2^13;2^13-1] | r limite [-2^13;2^13-1]
    static int calculerHash(const int q, const int r){ return (q << 14) | r; }
    std::vector<Point> surroundingPoints() const;
};

enum class TileType{Unknown, Default, Goal, Forbidden};

class Noeud {
public:
    const Point point;
    
private:
    TileType tiletype;
    std::vector<Noeud*> neighbours;
    std::set<const Mur*> murs;

public:
    // On delete le constructeur par défaut pour éviter les erreurs
    Noeud() = delete;
    
    Noeud(const Noeud& other) = default;
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

    const Noeud *getNeighbour(const EHexCellDirection &dir) const
    {
        for (auto neighbour : neighbours) {
        }
    }
    
    void addNeighbour(Noeud* neighbour)
    {
        for (auto mur : murs) {
            // Vérifier les murs
            if (mur->getNoeudOppose(this) == neighbour)
                return;
        }
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

    void addMur(const Mur *mur)
    {
        murs.insert(mur);
        const Noeud *oppose = mur->getNoeudOppose(this);
        auto p = std::find(neighbours.begin(), neighbours.end(), oppose);
        if (p != neighbours.end()) removeNeighbour(oppose); // TODO: vérification inutile si pas de throw
        // Peut-être remplacer neighbours par un set ?
    }
    
    EHexCellDirection getDirection(const Noeud& other) const;
    Point getPointNeighbour(EHexCellDirection cellDirection) const;
};

#endif // NOEUD_H
