#include "client.hpp"

void Client::startAsClient() {
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        cout << "(no-id): ERROR cannot get address info for host (" << hostname  << ", " << port << ")\n";
        throw ClientException();
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cout << "(no-id): ERROR cannot create socket (" << hostname  << ", " << port << ")\n";
        throw ClientException();
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cout << "(no-id): ERROR cannot connect to socket (" << hostname  << ", " << port << ")\n";
        throw ClientException();
    }
}