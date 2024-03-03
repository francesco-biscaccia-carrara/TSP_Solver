#include "distance.h"


int euc_2d(point a, point b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y; 

    return nint(sqrt((dx * dx) + (dy * dy)));
}

int man_2d(point a, point b) {
    double dx = abs(b.x - a.x);
    double dy = abs(b.y - a.x);

    return nint(dx + dy); 
}

int max_2d(point a, point b) {
    double dx = abs(b.x - a.x);
    double dy = abs(b.y - a.x);

    return (dx>=dy) ? dx : dy;
}