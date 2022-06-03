//
// Created by musht on 2022-06-03.
//

#include "data_scheme.h"
#include <stdio.h>

void playerToString(PlayerData *pd, char *dest) {
    sprintf(dest, "%d %s %s %d %d %d", pd->ID, pd->login, pd->password, pd->gamesPlayed, pd->wins, pd->highScore);
}

void playerFromStr(PlayerData *pd, char *src) {
    sscanf(src, "%d %s %s %d %d %d", &pd->ID, pd->login, pd->password, &pd->gamesPlayed, &pd->wins, &pd->highScore);
}