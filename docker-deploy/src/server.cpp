#include "server.hpp"

void Server::startAsServer() {
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cout << "(no-id): ERROR cannot get address info for host (" << hostname  << ", " << port << ")\n";
        throw ServerException();
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cout << "(no-id): ERROR cannot create socket (" << hostname  << ", " << port << ")\n";
        throw ServerException();
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cout << "(no-id): ERROR cannot bind socket (" << hostname  << ", " << port << ")\n";
        throw ServerException();
    }

    status = listen(socket_fd, 100);
    if (status == -1) {
        cout << "(no-id): ERROR cannot listen on socket (" << hostname  << ", " << port << ")\n";
        throw ServerException();
    }
}

int Server::acceptConnection(string *ip_addr){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    
    int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        cout << "(no-id): ERROR cannot accept connection on socket";
        throw ServerException();
    }

    *ip_addr = inet_ntoa(((struct sockaddr_in *)&socket_addr)->sin_addr);
    return client_connection_fd;
}