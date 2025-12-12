#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <WS2tcpip.h>
#include <cstdlib>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

class TcpServer final
{
private:
    SOCKET m_listeningSocket = INVALID_SOCKET;
    SOCKET m_clientSocket = INVALID_SOCKET;
    static constexpr size_t BUFFER_SIZE = 4096;
    static constexpr int PORT = 54000;

public:
    TcpServer()
    {
        WSAData data{};
        const WORD version = MAKEWORD(2, 2);

        if (const int result = WSAStartup(version, &data); result != 0) {
            throw runtime_error("Не удалось инициализировать Winsock, код ошибки: " + to_string(result));
        }

        m_listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_listeningSocket == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Не удалось создать сокет");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(m_listeningSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            closesocket(m_listeningSocket);
            WSACleanup();
            throw runtime_error("Ошибка привязки сокета, код: " + to_string(error));
        }

        if (listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            closesocket(m_listeningSocket);
            WSACleanup();
            throw runtime_error("Ошибка начала прослушивания, код: " + to_string(error));
        }

        cout << "TCP сервер ожидает подключения на порт " << PORT << "..." << endl;

        sockaddr_in clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        m_clientSocket = accept(m_listeningSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
        if (m_clientSocket == INVALID_SOCKET) {
            const int error = WSAGetLastError();
            closesocket(m_listeningSocket);
            WSACleanup();
            throw runtime_error("Ошибка принятия подключения, код: " + to_string(error));
        }

        closesocket(m_listeningSocket);
        m_listeningSocket = INVALID_SOCKET;

        char host[NI_MAXHOST] = { 0 };
        char service[NI_MAXSERV] = { 0 };

        if (getnameinfo(reinterpret_cast<sockaddr*>(&clientAddr), sizeof(clientAddr),
            host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            cout << "Клиент " << host << " подключился через порт " << service << endl;
        }
        else {
            inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
            cout << "Клиент " << host << " подключился через порт " << ntohs(clientAddr.sin_port) << endl;
        }
    }

    ~TcpServer() noexcept
    {
        if (m_clientSocket != INVALID_SOCKET) {
            closesocket(m_clientSocket);
        }
        if (m_listeningSocket != INVALID_SOCKET) {
            closesocket(m_listeningSocket);
        }
        WSACleanup();
    }

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    TcpServer(TcpServer&& other) noexcept
        : m_listeningSocket(other.m_listeningSocket), m_clientSocket(other.m_clientSocket)
    {
        other.m_listeningSocket = INVALID_SOCKET;
        other.m_clientSocket = INVALID_SOCKET;
    }

    TcpServer& operator=(TcpServer&& other) noexcept
    {
        if (this != &other) {
            if (m_clientSocket != INVALID_SOCKET) {
                closesocket(m_clientSocket);
            }
            if (m_listeningSocket != INVALID_SOCKET) {
                closesocket(m_listeningSocket);
            }
            m_listeningSocket = other.m_listeningSocket;
            m_clientSocket = other.m_clientSocket;
            other.m_listeningSocket = INVALID_SOCKET;
            other.m_clientSocket = INVALID_SOCKET;
        }
        return *this;
    }

    void run()
    {
        vector<char> buffer(BUFFER_SIZE);

        while (true) {
            buffer.assign(buffer.size(), 0);

            const int bytesReceived = recv(m_clientSocket, buffer.data(),
                static_cast<int>(buffer.size() - 1), 0);

            if (bytesReceived == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                cerr << "Ошибка при получении данных, код: " << error << endl;
                break;
            }

            if (bytesReceived == 0) {
                cout << "Клиент отключился" << endl;
                break;
            }

            const string msgReceived(buffer.data(), 0, bytesReceived);
            cout << "Клиент> " << msgReceived << endl;

            if (msgReceived.find("quit") == 0) {
                cout << "Получена команда выхода. Завершение работы сервера." << endl;
                break;
            }

            if (const int sendResult = send(m_clientSocket, buffer.data(), bytesReceived, 0);
                sendResult == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                cerr << "Ошибка отправки данных клиенту, код: " << error << endl;
                break;
            }
        }
    }
};

int main()
{
    system("chcp 1251>nul");
    try {
        TcpServer server;
        server.run();
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}