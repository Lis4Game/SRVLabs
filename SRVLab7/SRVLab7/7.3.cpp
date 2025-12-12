#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <WS2tcpip.h>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class UdpServer final
{
private:
    SOCKET m_socket = INVALID_SOCKET;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr int PORT = 54000;

public:
    UdpServer()
    {
        WSAData data{};
        const WORD version = MAKEWORD(2, 2);

        if (const int result = WSAStartup(version, &data); result != 0) {
            throw runtime_error("Не удалось инициализировать Winsock, код ошибки: " + to_string(result));
        }

        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Не удалось создать UDP-сокет");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            closesocket(m_socket);
            WSACleanup();
            throw runtime_error("Ошибка привязки сокета, код: " + to_string(error));
        }

        cout << "UDP сервер запущен на порту " << PORT << "..." << endl;
    }

    ~UdpServer() noexcept
    {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
        WSACleanup();
    }

    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

    UdpServer(UdpServer&& other) noexcept : m_socket(other.m_socket)
    {
        other.m_socket = INVALID_SOCKET;
    }

    UdpServer& operator=(UdpServer&& other) noexcept
    {
        if (this != &other) {
            if (m_socket != INVALID_SOCKET) {
                closesocket(m_socket);
            }
            m_socket = other.m_socket;
            other.m_socket = INVALID_SOCKET;
        }
        return *this;
    }

    void run()
    {
        vector<char> buffer(BUFFER_SIZE);
        sockaddr_in clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        while (true) {
            buffer.assign(buffer.size(), 0);
            ZeroMemory(&clientAddr, sizeof(clientAddr));

            const int bytesReceived = recvfrom(m_socket, buffer.data(),
                static_cast<int>(buffer.size() - 1), 0,
                reinterpret_cast<sockaddr*>(&clientAddr),
                &clientAddrSize);

            if (bytesReceived == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                cerr << "Ошибка при получении данных от клиента, код: " << error << endl;
                continue; 
            }

            char clientIp[INET_ADDRSTRLEN] = { 0 };
            if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN) == nullptr) {
                const unsigned long addr = clientAddr.sin_addr.s_addr;
                snprintf(clientIp, sizeof(clientIp), "%u.%u.%u.%u",
                    (addr >> 0) & 0xFF, (addr >> 8) & 0xFF,
                    (addr >> 16) & 0xFF, (addr >> 24) & 0xFF);
            }

            buffer[bytesReceived] = '\0'; 
            cout << "Сообщение от " << clientIp << " : " << buffer.data() << endl;
        }
    }
};

int main()
{
    system("chcp 1251>nul");
    try {
        UdpServer server;
        server.run();
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}