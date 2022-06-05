#include "game.h"

void initPlayer(Player *p) {
    p->y = p->x = 0;
    p->dx = p->dy = 0;
    p->onGround = 0;
    p->curFrame = 0;
}

void updatePlayer(Player *p, float t) {
    p->x += p->dx * t; // горизонтальное перемещение

    if (!p->onGround) // вертикальное перемещение
        p->dy += .00085f * t;
    p->y += p->dy * t;

    p->onGround = 0; // проверка на приземление
    if (p->y >= GROUND) {
        p->y = GROUND;
        p->dy = 0;
        p->onGround = 1;
    }
}

void walk(Player *p, Directions direction) {
    p->dx = (float) direction * 0.3f;
    p->dir = direction;
}

void leap(Player *p) {
    if (!p->onGround)
        return;
    p->dy = -0.7f;
    p->onGround = 0;
    p->curFrame = 0;
}