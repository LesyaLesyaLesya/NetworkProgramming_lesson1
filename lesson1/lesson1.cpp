#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <ws2tcpip.h>

#include "socket_headers.h"
#include "socket_wrapper.h"
#include "socket_class.h"


// Trim from end (in place).
static inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base());
    return s;
}

char* muResolvedNameFunction(sockaddr* addr)
{
    char errorresolvaeName{ '?' };

    //socklen_t addrlen;
    char hbuf[NI_MAXHOST];
    if (getnameinfo(addr, sizeof(addr), hbuf, sizeof(hbuf), nullptr, 0, NI_NAMEREQD))
        return &errorresolvaeName;
    else
        return hbuf;
}

int main(int argc, char const* argv[])
{

    /*if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }*/

    const char* inp_port = "1230";
   

    socket_wrapper::SocketWrapper sock_wrap;
    //const int port{ std::stoi(argv[1]) };
    const int port{ std::stoi(inp_port) };

    socket_wrapper::Socket sock = { AF_INET, SOCK_DGRAM, IPPROTO_UDP };

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr;
    
    addr.sin_family = PF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    char buffer[256];

    // socket address used to store client address
    struct sockaddr_in client_address = { 0 };
    socklen_t client_address_len = sizeof(sockaddr_in);
    ssize_t recv_len = 0;

    std::cout << "Running echo server...\n" << std::endl;
    char client_address_buf[INET_ADDRSTRLEN];

    while (true)
    {
        // Read content into buffer from an incoming client.
        recv_len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
            reinterpret_cast<sockaddr*>(&client_address),
            &client_address_len);         

        if (recv_len > 0)
        {
            buffer[recv_len] = '\0';
            std::cout
                << "Client with address "
                << inet_ntop(AF_INET, &client_address.sin_addr, client_address_buf, sizeof(client_address_buf) / sizeof(client_address_buf[0]))
                << ":" << ntohs(client_address.sin_port)
                << " resolved name "
                << muResolvedNameFunction(reinterpret_cast<sockaddr*>(&client_address))
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << buffer
                << "\n'''"
                << std::endl;
            //if ("exit" == buffer) run = false;
            //send(sock, &buf, readden, 0);

//           std::string command_string = {buffer, 0, len};
 //           rtrim(command_string);
 //           std::cout << command_string << std::endl;
            if (strcmp(buffer, "exit\n") == 0)
            {
                std::cout << std::endl <<"Echo server stopped...";
                //sock.close();
                break;
            }
// Send same content back to the client ("echo").
            sendto(sock, buffer, recv_len, 0, reinterpret_cast<const sockaddr*>(&client_address),
                client_address_len);
        }

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}