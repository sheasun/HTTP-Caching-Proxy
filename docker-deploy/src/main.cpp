#include "proxy.hpp"
#include "server.hpp"
#include "thread_info.hpp"
#include "log_manager.hpp"

#define PORT "12345"
#define LOG_FILE_PATH "/var/log/erss/proxy.log"

using namespace std;

int main(){
    int thread_id = 0;
    Server server(PORT);
    Cache cache(100);
    LogManager log_manager(LOG_FILE_PATH);

    while (1) {      
        /* Spawn a thread to handle a request. If the server can't set up connection with the current client, 
           it will just skip it and move on to the next one. */
        try {
            string ip_addr;
            int browser_fd = server.acceptConnection(&ip_addr);
            thread_id ++;
            ThreadInfo * info = new ThreadInfo(&cache, ip_addr, &log_manager, thread_id, browser_fd);
            pthread_t thread;
            pthread_create(&thread, NULL, processRequest, info);
        }
        catch (ServerException& e) {
            cout << e.what();
        }
    }
    return EXIT_SUCCESS;
}
