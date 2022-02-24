#include "socket.hpp"

using namespace std;

struct ServerException : public exception {
    const char * what () const throw () {
        return "(no-id): ERROR server exception\n";
    }
};

class Server : public Socket {
public:
    Server(const char * port) : Socket(port) { startAsServer(); }
    void startAsServer();
    /* Accept connection from client. */
    int acceptConnection(string * ip_addr);
};