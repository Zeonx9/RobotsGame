//
// Created by Шадрин Антон Альберт on 24.05.2022.
//

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include "sqlite/sqlite3.h"
#include <string.h>

int findCallback(void *data, int argc, char **argv, char **azColName){
    PlayerData* pd = (PlayerData*) data;
    pd->ID = atoi(argv[0]);
    strcpy(pd->login, argv[1]);
    strcpy(pd->password, argv[2]);
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
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char sql[250];

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sprintf(sql, "CREATE TABLE IF NOT EXISTS PLAYERS("
                 "LOGIN TEXT NOT NULL,"
                 "PASSWORD TEXT NOT NULL);"
                 "INSERT INTO PLAYERS(LOGIN, PASSWORD)"
                 "VALUES('%s', '%s')", login, password);


    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, printCallback, NULL, &zErrMsg);

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);

}

void findPlayer(char* login){
    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[250];
    PlayerData* pd = malloc(sizeof(PlayerData));
    pd->ID = -1;

    rc = sqlite3_open("../players", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(4);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }
    sprintf(sql,"SELECT * FROM PLAYERS WHERE LOGIN = '%s';", login);

    rc = sqlite3_exec(db, sql, findCallback, (void*) pd, &zErrMsg);
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }

    printf("%d", pd->ID);
    sqlite3_close(db);


}

