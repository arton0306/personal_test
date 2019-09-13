class Server {
 public:
    Server(int port, int max_clients);
    void run();
    
 private:
    int port_;
    int max_clients_;
};

class MonitoredFdMgr {
 public:
    MonitoredFdMgr();
    bool add(int fd);
    bool remove(int fd);
    int get_max_fd() const;
    *fd_set get_fdset();
    
 private:
    set<int> monitored_fd_set_;
    fd_set fds_;
};
