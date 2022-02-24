#ifndef _SOCKET_HPP__
#define _SOCKET_HPP__

#include <iostream>
#include <exception>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>

/* Reference: ECE.650 tcp_example. */
class Socket {
public:
    int status;
    int socket_fd;
    const char * port;
    const char * hostname;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    
    Socket(const char * _port) : port(_port), hostname(NULL), host_info_list(NULL) {}
    Socket(const char * _hostname, const char * _port) : port(_port), hostname(_hostname), host_info_list(NULL) {}
    ~Socket(){
        close(socket_fd);
        freeaddrinfo(host_info_list);
    }
};
#endif
