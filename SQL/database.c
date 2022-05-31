#include "database.h"
#include "sqlite/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * - callback функция для SQL запроса
 * - заполняет все поля для существующего в базе данных пользователя и
 * - сохраняет их в специальную структуру Pair, в которой первое поле -
 * - номер обрабатываемого пользователя, а второе - непосредственно массив с пользователями
 * - размер массива задаётся параметром count функции findBestPlayers()
 **/
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

/**
 * callback функция для SQL запроса
 * заполняет все поля для существующего в базе данных пользователя
 */
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

/**
 * callback функция, которая печатает в консоль все данные на найденного пользователя
 * **/
int printCallback(void *data, int argc, char **argv, char **azColName){
    int i;
    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/**
 * Функция для регистрации нового пользователя
 * Для регистрации нужны логин и пароль
 * Возвращает -1 при ошибке регистрации
 * **/
int registerUser(char * login, char * password){
    // User already exists
    if (findPlayer(login)->ID != -1){
        return -1;
    }
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char sql[600];

    // Открыть базу данных
    rc = sqlite3_open("../players", &db);

    // Если не открылась, то ошибка
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Создание SQL запроса и таблицы в БД, если она не существует
    sprintf(sql, "CREATE TABLE IF NOT EXISTS PLAYERS("
                 "'ID' INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "'LOGIN' TEXT UNIQUE DEFAULT 0 NOT NULL,"
                 "'PASSWORD' TEXT NOT NULL,"
                 "'GAMES' INTEGER DEFAULT 0,"
                 "'HIGH SCORE' INTEGER DEFAULT 0,"
                 "'WINS' INTEGER DEFAULT 0"
                 ");"
                 "INSERT INTO PLAYERS(LOGIN, PASSWORD)"
                 "VALUES('%s', '%s')", login, password);


    // Выполнить запрос
    rc = sqlite3_exec(db, sql, printCallback, NULL, &zErrMsg);

    // Если не удалось, то ошибка
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);

        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -1;
    }

    // Закрыть БД
    sqlite3_close(db);
    return 0;
}

/**
 * Поиск пользователя в БД и возврат данных о нём в виде структуры PlayerData
 * Если пользователя нет, то в ID будет -1
 * **/
PlayerData *findPlayer(char* login){
    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[100];
    // Выделение памяти на куче для возврата из функции
    PlayerData* pd = malloc(sizeof(PlayerData));
    pd->ID = -1;


    rc = sqlite3_open("../players", &db);
    // Если БД не открылась
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return pd;
    }
    // Создание SQL запроса
    sprintf(sql,"SELECT * FROM PLAYERS WHERE LOGIN = '%s';", login);

    rc = sqlite3_exec(db, sql, findCallback, (void*) pd, &zErrMsg);
    // Запрос выполнился некорректно
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    // Закрыть БД
    sqlite3_close(db);

    return pd;

}

/**
 * Функция для обновления определённых данных о пользователе
 * Принимает ID и одно из полей enum-а(аналогичного полю таблицы) и значение, на которое нужно заменить
 * При ошибке возвращает -1
 * **/
int updateData(int ID, Categories category, int value){

    char categoryNames[3][11] = {"GAMES", "HIGH SCORE", "WINS"};
    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[250];
    // Открытие БД
    rc = sqlite3_open("../players", &db);

    // Если не открылась
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Создание SQL запроса
    sprintf(sql,"UPDATE PLAYERS SET '%s' = %d WHERE ID = %d", categoryNames[category], value, ID);

    // Исполнение запроса
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    // Если не выполнился, то ошибка
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return -1;
    }
    // Закрыть БД
    sqlite3_close(db);
    return 0;
}

/**
 * Функция для нахождения топа игроков по характеристике HIGH SCORE
 * Размер выборки определяется count
 * При отсутствии достаточного количества пользователей, недостающие будут помечены при помощи
 * -1 в ID
 * Возвращает ссылку на массив с пользователями
 * **/
PlayerData **findBestPlayers(int count){

    sqlite3 *db;

    char *zErrMsg = 0;
    int rc;
    char sql[250];

    // Открыть БД
    rc = sqlite3_open("../players", &db);
    // Если не открылась
    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    PlayerData** pd = malloc(sizeof(PlayerData *) * count);
    // Заполнение всех ID по стандарту -1
    for (int i = 0; i < count; ++i) {
        pd[i] = malloc(sizeof(PlayerData));
        pd[i]->ID = -1;
    }
    Pair *pr = malloc(sizeof(Pair));
    // В i номер текущего пользователя для обработки
    int i = 0;
    pr->pd = pd;
    pr->number = &i;
    // Создание запроса
    sprintf(sql, "SELECT * FROM PLAYERS\n"
                 "ORDER BY \"HIGH SCORE\" DESC LIMIT %d", count);

    // Исполнение запроса
    rc = sqlite3_exec(db, sql, topScoreCallback, pr, &zErrMsg);
    // Если не выполнился
    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
    }
    // Закрытие БД
    sqlite3_close(db);

    return pd;
}

