
#ifndef ARALE_EXAMPLE_IDLECONNECTION_ECHO_H
#define ARALE_EXAMPLE_IDLECONNECTION_ECHO_H

#include <arale/net/TcpServer.h>

#include <boost/circular_buffer.hpp>

#include <unordered_set>

class EchoServer {
public:
    EchoServer(arale::net::EventLoop *, const arale::net::InetAddress &, int idleSeconds);
    void start();

private:
    void onMessage(const arale::net::TcpConnectionPtr &,
                    arale::net::Buffer *,
                    arale::Timestamp);
    void onConnection(const arale::net::TcpConnectionPtr &);
    void onTimer();
    void dumpConnectionBuckets();

    typedef std::weak_ptr<arale::net::TcpConnection> WeakTcpConnectionPtr;

    struct Entry {
        explicit Entry(const WeakTcpConnectionPtr &weakConn)
            : weakConn_(weakConn) {

        }

        // there will be only one Entry object for each Tcpconnection object
        ~Entry() {
            // can Entry use shared_ptr, and check the user counter here
            // if the user counter is big than 1, that means the conneciton
            // is expired, we need to shutdown the connection
            arale::net::TcpConnectionPtr conn = weakConn_.lock();
            if (conn) {
                conn->shutdown();
            }
        }
        
        WeakTcpConnectionPtr weakConn_;
    };
    
    // we need shared_ptr to implement RAII
    typedef std::shared_ptr<Entry>  EntryPtr;
    typedef std::unordered_set<EntryPtr> Bucket;
    typedef boost::circular_buffer<Bucket>  ConnectionBuckets;
    
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    
    arale::net::TcpServer server_;
    ConnectionBuckets connections_;
};

#endif
