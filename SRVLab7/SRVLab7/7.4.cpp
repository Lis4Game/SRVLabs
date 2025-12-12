#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <WS2tcpip.h>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class UdpClient final
{
private:
    SOCKET m_socket = INVALID_SOCKET;
    sockaddr_in m_serverAddr{};
    static constexpr int PORT = 54000;
    static constexpr const char* SERVER_IP = "127.0.0.1";

public:
    UdpClient()
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

        ZeroMemory(&m_serverAddr, sizeof(m_serverAddr));
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, SERVER_IP, &m_serverAddr.sin_addr) <= 0) {
            closesocket(m_socket);
            WSACleanup();
            throw runtime_error("Неверный IP-адрес сервера: " + string(SERVER_IP));
        }

        cout << "UDP клиент готов. Введите сообщение." << endl;
    }

    ~UdpClient() noexcept
    {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
        WSACleanup();
    }

    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;

    UdpClient(UdpClient&& other) noexcept : m_socket(other.m_socket), m_serverAddr(other.m_serverAddr)
    {
        other.m_socket = INVALID_SOCKET;
        ZeroMemory(&other.m_serverAddr, sizeof(other.m_serverAddr));
    }

    UdpClient& operator=(UdpClient&& other) noexcept
    {
        if (this != &other) {
            if (m_socket != INVALID_SOCKET) {
                closesocket(m_socket);
            }
            m_socket = other.m_socket;
            m_serverAddr = other.m_serverAddr;
            other.m_socket = INVALID_SOCKET;
            ZeroMemory(&other.m_serverAddr, sizeof(other.m_serverAddr));
        }
        return *this;
    }

    void run()
    {
        string userInput;

        while (true) {
            cout << "> ";
            if (!getline(cin, userInput)) {
                break; 
            }

            if (userInput == "quit") {
                cout << "Завершение работы..." << endl;
                break;
            }

            const int sendResult = sendto(m_socket, userInput.c_str(),
                static_cast<int>(userInput.length()), 0,
                reinterpret_cast<sockaddr*>(&m_serverAddr),
                sizeof(m_serverAddr));

            if (sendResult == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                cerr << "Ошибка отправки данных, код: " << error << endl;
            }
        }
    }
};

int main()
{
    system("chcp 1251>nul");
    try {
        UdpClient client;
        client.run();
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}