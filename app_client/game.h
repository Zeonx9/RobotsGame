#ifndef ROBOTSGAME_GAME_H
#define ROBOTSGAME_GAME_H

#define WIDTH 80
#define HEIGHT 120
#define TILE 60

typedef enum directions {
    Left = -1, Right = 1
} Directions;

typedef struct bullet {
    float dir, x, y;
}Bullet;

typedef struct player {
    float dx, dy, y, x, dir;
    int onGround, jumped;
} Player;


void initPlayer(Player *p);
void updatePlayer(Player *p, float t, char ** field);

void initBullet(Player *p, Bullet *bullets);

void walk(Player *p, Directions direction);
void leap(Player *p);

#endif //ROBOTSGAME_GAME_H
