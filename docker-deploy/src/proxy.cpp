#include "proxy.hpp"

string currTime() {
    /* Change time to UTC-5. */
    time_t currTime = time(0) - 18000;
    struct tm * nowTime = gmtime(&currTime);
    const char * t = asctime(nowTime);
    string tme(t);
    return tme.substr(0, tme.size() - 1);
}

void cacheResponse(Cache * cache, string res_str, string uri, int thread_id, LogManager * log_manager) {
    Response res(res_str);
    string code = res.getCode();
    /* Check if the response can be stored in cache. */
    if (code == "200" && !res.isNoStore() && !res.isPrivate()) {
        cache->put(uri, res.getResponse());
        string msg, expire = res.getExpires();
        if (expire == "") {
            msg = to_string(thread_id) + ": cached, but requires re-validation\n";   
        }
        else {
            msg = to_string(thread_id) + ": cached, expires at " + expire + "\n";
        }
        cout << msg;
        log_manager->writeLog(msg);
    }
    else {
        string reason;
        if (code != "200") {
            reason = "code " + code;
        }
        else if (res.isNoStore()) {
            reason = "no-store";
        }
        else if (res.isPrivate()) {
            reason = "private";
        }
        string msg = to_string(thread_id) + ": not cacheable because " + reason + "\n";
        cout << msg;
        log_manager->writeLog(msg);
    }
}

Response getGETResponse(int fd) {
    vector<char> buf(BUF_SIZE);
    int index = recv(fd, &buf.data()[0], buf.size(), 0);
    Response res = Response(buf);
    if (res.getCode() == "304") {
        return res;
    }

    if (res.isChunked()) {
        /* Keep receive data until chunk ends. */
        string chunk = res.getResponse();
        /* Ends with a packet of size 0. */
        while (chunk.find("0\r\n\r\n") == string::npos) {
            buf.resize(index + BUF_SIZE);
            int len = recv(fd, &buf.data()[index], BUF_SIZE, 0);
            if (len <= 0) {
                break;
            }
            chunk = "";
            chunk.insert(chunk.begin(), buf.begin() + index, buf.begin() + index + len);
            index += len;
        }
    }
    else {
        /* Keep receive data until all have been collected. */
        int content_len = res.getContentLength();
        while (index < content_len) {
            buf.resize(index + BUF_SIZE);
            int len = recv(fd, &buf.data()[index], buf.size(), 0);
            if (len <= 0){
                break;
            }
            index += len;
        }
    }
    res = Response(buf);
    return res;
}

void processPOST(Request request, ThreadInfo * info, int server_fd) {
    int thread_id = info->thread_id;
    int client_fd = info->browser_fd;
    string ip_addr = info->ip_addr;
    LogManager * log_manager = info->log_manager;

    int content_len = request.getContentLen();
    if (content_len != -1) {
        string req_content = request.getRequest();
        char req[req_content.length() + 1];
        strcpy(req, req_content.c_str());

        string msg = to_string(thread_id) + ": Requesting \"" + request.getLine() + "\" from " + ip_addr + "\n";
        cout << msg;
        log_manager->writeLog(msg);
        send(server_fd, req, sizeof(req), MSG_NOSIGNAL);

        char buf[BUF_SIZE] = { 0 };
        int res_len = recv(server_fd, buf, sizeof(buf), MSG_WAITALL);
        if (res_len != 0) {
            Response res(buf);
            send(client_fd, buf, res_len, MSG_NOSIGNAL);
            msg = to_string(thread_id) + ": Responding \"" + res.getFirstLine() + "\"\n";
            cout << msg;
            log_manager->writeLog(msg);
        }
        else {
            msg = to_string(thread_id) + ": ERROR POST Failure @ " + currTime() + "\n";
            cout << msg;
            log_manager->writeLog(msg);
        }
    }
    return;
}

string revalidate(Cache * cache, Request request, Response response, int server_fd, int thread_id) {
    if (response.getEtag() == "" && response.getLastModified() == "") {
        return response.getResponse();
    }
    if(response.getEtag() != ""){
        string origin = request.getHeader();
        string newRequest = origin + "\r\n" + "If-None-Match: " + response.getEtag() + "\r\n\r\n";

        vector<char> buf(BUF_SIZE);
        int index = recv(server_fd, &buf.data()[0], buf.size(), 0);
        Response updateRes(buf);
        if (updateRes.getCode() == "304") {
            return response.getResponse();
        }
        else {
            if (!updateRes.isPrivate() && !updateRes.isNoStore() && updateRes.getCode() == "200") {
                cache->put(request.getURI(), updateRes.getResponse());
            }
            return updateRes.getResponse();
        }
    }
    else if (response.getLastModified() != "") {
        string origin=request.getHeader();
        string newRequest = origin + "\r\n" + "If-Modified-Since: " + response.getLastModified() + "\r\n\r\n";

        vector<char> buf(BUF_SIZE);
        int index = recv(server_fd, &buf.data()[0], buf.size(), 0);
        Response updateRes(buf);
        if (updateRes.getCode() == "304") {
            return response.getResponse();
        }
        else{
            if (!updateRes.isPrivate() && !updateRes.isNoStore() && updateRes.getCode() == "200") {
                cache->put(request.getURI(), updateRes.getResponse());
            }
            return updateRes.getResponse();
        }
    }
    else {
        string origin=request.getRequest();
        vector<char> buf(BUF_SIZE);
        int index = recv(server_fd, &buf.data()[0], buf.size(), 0);
        Response updateRes(buf);
        if (!updateRes.isPrivate() && !updateRes.isNoCache() && updateRes.getCode() == "200") {
            cache->put(request.getURI(), updateRes.getResponse());
        }
        return updateRes.getResponse();
    }
}

void processGET(Request req, ThreadInfo * info, int server_fd) {
    Cache * cache = info->cache;
    int thread_id = info->thread_id;
    int client_fd = info->browser_fd;
    string ip_addr = info->ip_addr;
    LogManager * log_manager = info->log_manager;

    string uri = req.getURI();
    string res_str = cache->get(uri);
    /* Check if the request has already been stored in cache. */
    if (res_str == "") {
        string msg = to_string(thread_id) + ": not in cache\n";
        cout << msg;
        log_manager->writeLog(msg);
        
        /* Send request to server. */
        send(server_fd, req.getRequest().data(), req.getRequest().size() + 1, 0);
        msg = to_string(thread_id) + ": Requesting \"" + req.getLine() + "\" from " + ip_addr + " @ " + currTime() + "\n";
        cout << msg;
        log_manager->writeLog(msg);

        Response res = getGETResponse(server_fd);
        msg = to_string(thread_id) + ": Received \"" + res.getFirstLine() + "\" from " + ip_addr + "\n";
        cout << msg;
        log_manager->writeLog(msg);

        send(client_fd, res.getResponse().data(), res.getResponse().size(), 0);
        msg = to_string(thread_id) + ": Responding \"" + res.getFirstLine() + "\"\n";
        cout << msg;
        log_manager->writeLog(msg);

        cacheResponse(cache, res.getResponse(), req.getURI(), thread_id, log_manager);
    }
    else {
        Response res = Response(res_str);
        if (res.needRevalidate(thread_id, log_manager)) {
            string new_res = revalidate(cache, req, res, server_fd, thread_id);
            send(client_fd, new_res.data(), new_res.size(), 0);
            string msg = to_string(thread_id) + ": in cache, requires validation\n";
            cout << msg;
            log_manager->writeLog(msg);
            msg = to_string(thread_id) + ": Responding \"" + Response(new_res).getFirstLine() + "\"\n";
            cout << msg;
            log_manager->writeLog(msg);
        }
        else {
            send(client_fd, res_str.data(), res_str.size(), 0);
            string msg = to_string(thread_id) + ": in cache, valid\n";
            cout << msg;
            log_manager->writeLog(msg);
            msg = to_string(thread_id) + ": Responding \"" + res.getFirstLine() + "\"\n";
            cout << msg;
            log_manager->writeLog(msg);
        }
    }
}

void processCONNECT(ThreadInfo * info, int server_fd) {
    int thread_id = info->thread_id;
    int client_fd = info->browser_fd;
    LogManager * log_manager = info->log_manager;

    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    string msg = to_string(thread_id) + ": Responding \"HTTP/1.1 200 OK\"\n";
    cout << msg;
    log_manager->writeLog(msg);
    
    /* Bridge between server and client. */
    fd_set fds;
    int nfds = max(server_fd, client_fd) + 1;
    int fds_arr[2] = { server_fd, client_fd };

    while (1) {
        FD_ZERO(&fds);
        for (int i = 0; i < 2; i ++) {
            FD_SET(fds_arr[i], &fds);
        }
        select(nfds, &fds, NULL, NULL, NULL);
    
        int len = 0;
        for (int i = 0; i < 2; i ++) {
            if (FD_ISSET(fds_arr[i], &fds)) {
                char buf[BUF_SIZE] = { 0 };
                len = recv(fds_arr[i], buf, sizeof(buf), 0);
                /* Connection ends. */
                if (len <= 0) {
                    return;
                } 
                if (send(fds_arr[1 - i], buf, len, 0) <= 0) {
                    return;
                }
            }
        }
    }
}

/* The entry of request handling. */
void * processRequest(void * thread_info) {
    ThreadInfo * info = (ThreadInfo *)thread_info;
    LogManager * log_manager = info->log_manager;
    string ip_addr = info->ip_addr;
    int thread_id = info->thread_id;
    int browser_fd = info->browser_fd;
    
    char buf[BUF_SIZE] = { 0 };
    int len = recv(browser_fd, buf, sizeof(buf), 0);
    /* Malformed request. */
    if (len <= 0) { 
        string msg = "(no-id): ERROR Malformed request from " + ip_addr + " @ " + currTime() + "\n";
        cout << msg;
        log_manager->writeLog(msg);
        return NULL;
    }

    Request req(string(buf, len));
    
    /* Check if the method is one of 'CONNECT', 'GET', and 'POST'. */
    if (req.check()) {
        string msg = to_string(thread_id) + ": \"" + req.getLine() + "\" from " + ip_addr + " @ " + currTime() + "\n";
        cout << msg;
        log_manager->writeLog(msg);
    }
    else {
        string msg = to_string(thread_id) + ": \"" + "Responding HTTP/1.1 400 Bad Request\n";
        cout << msg << endl;
        log_manager->writeLog(msg);
        /* Inform browser. */
        send(browser_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
        return NULL;
    }

    try {
        /* Connect with server. */
        Client client(req.getHost().c_str(), req.getPort().c_str());

        /* Process Request according to its method. */
        string method = req.getMethod();
        if (method == "GET") {
            processGET(req, info, client.socket_fd);
        }
        else if (method == "POST") {
            processPOST(req, info, client.socket_fd);
        }
        else if (method == "CONNECT") {
            processCONNECT(info, client.socket_fd);
            string msg = to_string(thread_id) + ": Tunnel closed\n";
            cout << msg;
            log_manager->writeLog(msg);
        }
        return NULL;
    }
    catch (ClientException& e) {
        cout << e.what();
        return NULL;
    }
}