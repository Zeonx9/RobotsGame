//
// Created by Шадрин Антон Альберт on 24.05.2022.
//

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include "sqlite/sqlite3.h"
#include <string.h>


int topScoreCallback(void *data, int argc, char **argv, char **azColName){
    Pair *pr = (Pair*)data;
    int *i = pr->number;
    PlayerData *pd = pr->pd[*i];
    pd->ID = atoi(argv[0]);
    strcpy(pd->login, argv[1]);
    strcpy(pd->password, argv[2]);
    pd->gamesPlayed = atoi(argv[3]);
    pd->highScore = atoi(argv[4]);
    pd->wins = atoi(argv[5]);
    (*i)++;
    return 0;
}

int findCallback(void *data, int argc, char **argv, char **azColName){
    PlayerData* pd = (PlayerData*) data;
    pd->ID = atoi(argv[0]);
    strcpy(pd->login, argv[1]);
    strcpy(pd->password, argv[2]);
    pd->gamesPlayed = atoi(argv[3]);
    pd->highScore = atoi(argv[4]);
    pd->wins = atoi(argv[5]);
    return 0;
}

int printCallback(void *data, int argc, char **argv, char **azColName){
    int i;


    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

void registerUser(char * login, char * password){
    // User already exists
    if (findPlayer(login)->ID != -1){
        exit(7);
    }
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char sql[250];

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    }

    /* Create SQL statement */
    sprintf(sql, "CREATE TABLE IF NOT EXISTS PLAYERS("
                 "'ID' INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "'LOGIN' TEXT UNIQUE NOT NULL,"
                 "'PASSWORD' TEXT NOT NULL,"
                 "'GAMES' INTEGER,"
                 "'HIGH SCORE' INTEGER,"
                 "'WINS' INTEGER"
                 ");"
                 "INSERT INTO PLAYERS(LOGIN, PASSWORD)"
                 "VALUES('%s', '%s')", login, password);


    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, printCallback, NULL, &zErrMsg);

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);

}

PlayerData *findPlayer(char* login){
    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[60];
    PlayerData* pd = malloc(sizeof(PlayerData));
    pd->ID = -1;

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    }
    sprintf(sql,"SELECT * FROM PLAYERS WHERE LOGIN = '%s';", login);

    rc = sqlite3_exec(db, sql, findCallback, (void*) pd, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);

    return pd;

}

void updateData(int ID, Categories category, int value){

    char categoryNames[3][11] = {"GAMES", "HIGH SCORE", "WINS"};
    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[250];

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    }

    sprintf(sql,"UPDATE PLAYERS SET '%s' = %d WHERE ID = %d", categoryNames[category], value, ID);

    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

PlayerData **findBestPlayers(int count){

    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[250];

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    }

    PlayerData** pd = malloc(sizeof(count));
    for (int i = 0; i < count; ++i) {
        pd[i] = malloc(sizeof(PlayerData));
    }
    int i = 0;
    Pair *pr = malloc(sizeof(Pair));
    pr->pd = pd;
    pr->number = &i;

    sprintf(sql, "SELECT * FROM PLAYERS\n"
                 "ORDER BY \"HIGH SCORE\" DESC LIMIT %d", count);

    rc = sqlite3_exec(db, sql, topScoreCallback, pr, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    sqlite3_close(db);

    return pd;
}

