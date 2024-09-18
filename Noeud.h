struct Point {
    int q;
    int r;

    Point() = default;

    Point (const int q, const int r) : q(q), r(r) {}
    float calcul_Distance(const Point& destination) const;

    bool operator==(const Point& other) const {
        return q == other.q && r == other.r;
    }

    bool operator!=(const Point &other) const {
        return !(*this == other);
    }
    static float calcul_Distance_Coordonnes(int q1, int r1, int q2, int r2);
};

public class Noeud
{
    
}