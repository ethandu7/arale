
#ifndef ARALE_EXAMPLE_IDLECONNECTION_ECHO_H
#define ARALE_EXAMPLE_IDLECONNECTION_ECHO_H

#include <arale/net/TcpServer.h>

#include <boost/circular_buffer.hpp>

#include <unordered_set>

//#include <stdio.h>

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

// gcc 4.7 is the first version to support all features of C++11 
#if GCC_VERSION < 40700

namespace std {

// partial specialization for shared_ptr, which will be used in unordered_set
template<typename T>
class hash<shared_ptr<T> >  : public unary_function<shared_ptr<T>, size_t> {
public:
    // do NOT forget the const qualifier
    size_t operator () (const std::shared_ptr<T> &ptr) const {
        return hash<T *>()(ptr.get());
    }
};

}

#endif
    

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
        
        /*
        ~Entry() {
            long users = weakConn_.use_count();
            printf("[Entry destructor] %ld\n", users);
            if (users > 1) {
                weakConn_->shutdown();
            }
        }

        arale::net::TcpConnectionPtr weakConn_;
        */
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
