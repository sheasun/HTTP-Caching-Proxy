#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "cache.hpp"
#include "client.hpp"
#include "request.hpp"
#include "response.hpp"
#include "thread_info.hpp"

#define BUF_SIZE 80000

void * processRequest(void * thread_info);