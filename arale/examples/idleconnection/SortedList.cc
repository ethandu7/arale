
#include <arale/net/TcpConnection.h>
#include <arale/net/EventLoop.h>
#include <arale/net/TcpServer.h>

#include <assert.h>
#include <stdio.h>
#include <list>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

class EchoServer {
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, int idleSeconds)
        : server_(loop, "Echo Server", addr),
          idleSeconds_(idleSeconds) {
        using namespace std::placeholders;
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
        loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
        dumpConnectionList();
    }

    void start() {
        server_.start();
    }
    
private:
    typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
    typedef std::list<WeakTcpConnectionPtr> WeakConnectionList;
    
    struct Node {
        Timestamp lastReceiveTime_;
        WeakConnectionList::iterator pos_;
    };
    
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
        std::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->getName() << " echo " << msg.size()
                 << " bytes received at " << receiveTime.toString();
        conn->send(msg);
        assert(!conn->getContext().empty());
        Node *node = boost::any_cast<Node>(conn->getMutableContext());
        node->lastReceiveTime_ = Timestamp::now();
        // no iterators become invalidated
        connections_.splice(connections_.end(), connections_, node->pos_);
        assert(node->pos_ == --connections_.end());
        dumpConnectionList();
    }
    
    void onConnection(const arale::net::TcpConnectionPtr &conn) {
        LOG_INFO << "Echo Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

        if (conn->isConnected()) {
            Node node;
            node.lastReceiveTime_ = Timestamp::now();
            connections_.push_back(conn);
            // we don't have to worry about iterator invalidation
            // cause we are using list, insertion has no affcet on all iterators
            // erasure only invalidate the itrator point to the element being erased
            node.pos_ = --connections_.end();
            conn->setContext(node);
        } else {
            assert(!conn->getContext().empty());
            const Node &node = boost::any_cast<const Node &>(conn->getContext());
            connections_.erase(node.pos_);
        }
        dumpConnectionList();
    }
    
    void onTimer() {
        dumpConnectionList();
        for (auto it = connections_.begin(); it != connections_.end(); ) {
            TcpConnectionPtr conn = it->lock();
            if (conn) {
                assert(!conn->getContext().empty());
                // it's very wired that the template argument is not 'Node *' but 'Node'
                Node *node = boost::any_cast<Node>(conn->getMutableContext());
                double age = timeDifference(Timestamp::now(), node->lastReceiveTime_);
                if (age > idleSeconds_) {
                    if (conn->isConnected()) {
                        conn->shutdown(); 
                        LOG_INFO << "shutting down " << conn->getName();
                        // > round trip of the whole Internet.
                        conn->forceCloseWithDelay(3.5);
                    }
                } else if (age < 0) {
                    LOG_INFO << "Time jump";
                    node->lastReceiveTime_ = Timestamp::now();
                } else {
                    // once we got a connection is not expired, the connections
                    // in the list after current one are all not expired
                    // we can skip them
                    break;
                }
                ++it;
            } else {
                LOG_INFO << "Expired";
                // it will point to next connection or end()
                it = connections_.erase(it);
            }
        }
    }
    
    void dumpConnectionList() {
        LOG_INFO << "size = " << connections_.size();

        for (auto it = connections_.begin(); it != connections_.end(); ++it) {
            TcpConnectionPtr conn = it->lock();
            if (conn) {
                printf("conn %p\n", conn.get());
                const Node &node = boost::any_cast<const Node &>(conn->getContext());
                printf("    time %s\n", node.lastReceiveTime_.toString().c_str());
            } else {
                printf("expired\n");
            }
        }
    }
    
    TcpServer server_;
    int idleSeconds_;
    WeakConnectionList connections_;
};

int main(int argc, char *argv[]) {
    EventLoop loop;
    InetAddress addr(2016);
    int idleSeconds = 10;
    if (argc > 1) {
        idleSeconds = atoi(argv[1]);
    }
    LOG_INFO << "pid = " << getCurrentThreadID()
             << ", idle seconds = " << idleSeconds;
    EchoServer server(&loop, addr, idleSeconds);
    server.start();
    loop.loop();
}

