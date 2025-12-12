#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <WS2tcpip.h>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

class TcpClient final
{
private:
    SOCKET m_socket = INVALID_SOCKET;
    static constexpr size_t BUFFER_SIZE = 4096;

public:
    explicit TcpClient(const string& ipAddress, int port)
    {
        WSAData data{};
        const WORD version = MAKEWORD(2, 2);

        if (const int result = WSAStartup(version, &data); result != 0) {
            throw runtime_error("Не удалось инициализировать Winsock, код ошибки: " + to_string(result));
        }

        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) {
            WSACleanup();
            throw runtime_error("Не удалось создать сокет");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(static_cast<u_short>(port));

        if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr) <= 0) {
            closesocket(m_socket);
            WSACleanup();
            throw runtime_error("Неверный IP-адрес");
        }

        if (const int result = connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)); result == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            closesocket(m_socket);
            WSACleanup();
            throw runtime_error("Не удалось подключиться к серверу, код ошибки: " + to_string(error));
        }
    }

    ~TcpClient() noexcept
    {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
        }
        WSACleanup();
    }

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    TcpClient(TcpClient&& other) noexcept : m_socket(other.m_socket)
    {
        other.m_socket = INVALID_SOCKET;
    }

    TcpClient& operator=(TcpClient&& other) noexcept
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
        string userInput;

        cout << "Подключено к серверу. Введите 'quit' для выхода.\n";

        while (true) {
            cout << "> ";
            if (!getline(cin, userInput)) {
                break; 
            }

            if (userInput.empty()) {
                continue;
            }

            const int sendResult = send(m_socket, userInput.c_str(),
                static_cast<int>(userInput.length() + 1), 0);

            if (sendResult == SOCKET_ERROR) {
                const int error = WSAGetLastError();
                cerr << "Ошибка отправки данных, код: " << error << endl;
                break;
            }

            buffer.assign(buffer.size(), 0);
            const int bytesReceived = recv(m_socket, buffer.data(),
                static_cast<int>(buffer.size() - 1), 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                cout << "СЕРВЕР> " << buffer.data() << endl;
            }
            else if (bytesReceived == 0) {
                cout << "Соединение закрыто сервером." << endl;
                break;
            }
            else {
                const int error = WSAGetLastError();
                cerr << "Ошибка получения данных, код: " << error << endl;
                break;
            }

            if (userInput == "quit") {
                cout << "Завершение работы..." << endl;
                break;
            }
        }
    }
};

int main()
{
    system("chcp 1251>nul");
    try {
        const string ipAddress = "127.0.0.1";
        const int port = 54000;

        TcpClient client(ipAddress, port);
        client.run();
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}