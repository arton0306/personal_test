#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory>
#include <errno.h>
#include <set>

using namespace std;

// ========================================================
// Server
// ========================================================
Server::Server(int port, int max_clients) 
    : port_(port)
    , max_clients_(max_clients)
{
    if (! (0 <= port && port <= 65535)) {
        throw std::invalid_argument("invalid port number");
    }
}

void Server::run()
{
    const int master_socket_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (master_socket_tcp_fd == -1) {
        throw "socket creation failed";
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port_;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(master_socket_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        throw "socket bind failed";
    }

    if (listen(master_socket_tcp_fd, max_clients_) < 0) {
        throw "listen failed";
    }

    MonitoredFdMgr monitoredFdMgr;
    monitoredFdMgr.add(master_socket_tcp_fd);

    while (true) {
        cout << "blocked on select system call..." << endl;

        fd_set *fdset = monitoredFdMgr.get_fdset();

        if (FD_ISSET(master_socket_tcp_fd, fdset)) {
            cout << "New connection received!" << endl;

            sockaddr_in client_addr;
            int addr_len;
            int comm_socket_fd = accept(master_socket_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
        }
    }
}

// ========================================================
// MonitoredFdMgr
// ========================================================
MonitoredFdMgr::MonitoredFdMgr()
{
}

bool MonitoredFdMgr::add(int fd)
{
    const auto r = monitored_fd_set_.insert(fd);
    return r.second;
}

bool MonitoredFdMgr::remove(int fd)
{
    const int erase_count = monitored_fd_set_.erase(fd);
    return erase_count > 0;
}

int MonitoredFdMgr::get_max_fd() const
{
    if (monitored_fd_set_.empty()) return -1;
    return *max_element(monitored_fd_set_.begin(), monitored_fd_set_.end());
}

*fd_set MonitoredFdMgr::get_fdset()
{
    FD_ZERO(&fds_);
    for (int fd: monitored_fd_set_) {
        FD_SET(fd, &fds_);
    }
    return &fds_;
}

