#include "socket.hpp"

using namespace std;

struct ClientException : public exception {
    const char * what () const throw () {
        return "(no-id): ERROR client exception\n";
    }
};

class Client : public Socket {
public:
    Client(const char * hostname, const char * port) : Socket(hostname, port) { startAsClient(); }
    /* Try to connect with server. */
    void startAsClient();
};