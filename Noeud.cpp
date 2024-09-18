#include "Noeud.cpp"


float Point::calcul_Distance(const Point &destination) const {
    return (abs(destination.q - q) 
          + abs(destination.q + destination.r - q - r)
          + abs(destination.r - r)) / 2;
}

float Point::calcul_Distance_Coordonnes(int q1, int r1, int q2, int r2) {
    Point p1 = Point(q1, r1);
    Point p2 = Point(q2, r2);
    return p1.calcul_Distance(p2);
}
