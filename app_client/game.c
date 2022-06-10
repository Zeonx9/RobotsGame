#include "game.h"

void initPlayer(Player *p) {
    p->y = p->x = 60;
    p->dx = p->dy = 0;
    p->onGround = p->jumped = 0;
}

void initBullet(Player *p, Bullet *bullets){
//    p->wasShoot = 1;
    for (int i = 0; i < 10; ++i) {
        if (bullets[i].dir == 0){
            bullets[i].dir = p->dir;
            bullets[i].x = p->x;
            bullets[i].y = p->y;
            break;
        }
    }
}

void collisionY(Player *p, char ** field) {
    p->onGround = 0;
    int i = (p->y + HEIGHT) / TILE;
    for (int j = p->x / TILE; j < (p->x + WIDTH) / TILE; ++j) {
        if (field[i][j] > '0'){
            p->y = i * TILE - HEIGHT;
            p->dy = 0;
            p->onGround = 1;
            return;
        }
    }
    i = p->y / TILE;
    for (int j = p->x / TILE; j < (p->x + WIDTH) / TILE; ++j) {
        if (field[i][j] > '0') {
            p->y = (i + 1) * TILE;
            p->dy = 0;
        }
    }
}

void collisionX(Player *p, char **field) {
    for (int i = (int)p->y / TILE; i < ((int)p->y + HEIGHT) / TILE; ++i)
        for (int j = (int)p->x / TILE; j <= ((int)p->x + WIDTH) / TILE; ++j) {
            if (field[i][j] > '0') {
                if (p->dir > 0) p->x = j * TILE - WIDTH;
                if (p->dir < 0) p->x = (j + 1) * TILE;
            }
        }
}

void updatePlayer(Player *p, float t, char **field) {
    p->x += p->dx * t; // горизонтальное перемещение
    collisionX(p, field);

    if (!p->onGround) // вертикальное перемещение
        p->dy += .00085f * t;
    p->y += p->dy * t;

    collisionY(p, field);
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
    p->jumped = 1;
}