#include <stdio.h>
#include <winsock2.h>

int CreateServer(){
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    // создать сокет для сервера (tcp/ip) проверить на нормальное создание, иначе выход
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server == INVALID_SOCKET) {
        printf("СANNOT CREATE SERVER SOCKET");
        return 1; // код возврата при невозможности создать сокет сервера
    } printf("Server created\n");

    // настройка сокета сервера: указание домена, адреса и порта
    serverAddr.sin_family = AF_INET; // домен интернета
    serverAddr.sin_port = htons(2205); // номер порта (22 год май месяц)
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // любой адрес

    // привязывание сокету сервера сформированного адреса и проверка на удачную привязку
    if (bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("CANNOT BIND SERVER ADDRESS");
        return 2; // код возврата при невозможности закрепить адрес за сервером а
    } printf("Server bound\n");

    // начинаем прослушивание, 50 - максимальное число клиентов в ожидании на подключение
    if (listen(server, 5) == SOCKET_ERROR) {
        printf("CANNOT START TO LISTEN");
        return 3; // код возврата при невозможности инициализировать прослушивание
    } printf("Server started to listen\n");

    // бесконечный цикл - сервер работает и готов устанавливать соединения с клиентами
    while (1) {
        int size = sizeof(clientAddr);
        // захват клиента и установление соединения
        client = accept(server, (SOCKADDR *) &clientAddr, &size);
        if (client == INVALID_SOCKET) {
            printf("ERROR ACCEPT CLIENT\n");
            continue; // попробуем снова установить соединение
        }
        printf("Client accepted\n");

        ///
        /// here can send and receive data to/from client
        ///
        break;
    }

    printf("Server stopped\n");
    closesocket(server);
    getchar();
    return 0;
}

int main() {
    WSADATA wsd;

    // подключение библиотеки ws2_32.lib
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("CANNOT CONNECT TO THE LIB");
        return 1;
    } printf("Connected to the lib\n");

    // запуск сервера
    return CreateServer();
}
