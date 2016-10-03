
#include "Echo.h"

#include <arale/net/EventLoop.h>

#include <assert.h>
#include <stdio.h>

using namespace arale;
using namespace arale::net;
using namespace arale::base;

EchoServer::EchoServer(EventLoop *loop, const InetAddress &addr, int idleConnneciton) 
    : server_(loop, "Echo Server", addr),
      connections_(idleConnneciton) {
    using namespace std::placeholders;
    server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
    loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
    connections_.resize(idleConnneciton);
    dumpConnectionBuckets();
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const TcpConnectionPtr &conn) {
    LOG_INFO << "Echo Server - " << conn->remoteAddr().toIpPort() << " -> "
             << conn->localAddr().toIpPort() << " is "
             << (conn->isConnected() ? "UP" : "DOWN");

    if (conn->isConnected()) {
        EntryPtr entry(new Entry(conn));
        // start to monitor this connection
        connections_.back().insert(entry);
        dumpConnectionBuckets();
        // must use weak_ptr<Entry> here
        // suppose we use shared_ptr<Entry>, then the Tcpconnection will 
        // have a strong reference to Entry object, which would make Entry object
        // still be there even the connection is expired, then accidently extend
        // the left time of conneciton
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
    } else {
        assert(!conn->getContext().empty());
        WeakEntryPtr weakEntry = boost::any_cast<WeakEntryPtr>(conn->getContext());
        LOG_INFO << "Entry use_count = " << weakEntry.use_count();
    }
}

void EchoServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp receiveTime) {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->getName() << " echo " << msg.size()
             << " bytes received at " << receiveTime.toString();
    conn->send(msg);
    assert(!conn->getContext().empty());
    WeakEntryPtr weakEntry = boost::any_cast<WeakEntryPtr>(conn->getContext());
    EntryPtr entry(weakEntry.lock());
    if (entry) {
        // this will make the reference of Entry object increase one
        // which extend the left time of Entry object, in turn will 
        // extend the lef time of connection
        connections_.back().insert(entry);
        dumpConnectionBuckets();
    }
}

void EchoServer::onTimer() {
    // when insert a Bucket object to the end of circular list
    // the first Bucket object in the list will be popped out, 
    // and that Bucket object will be destructed, the EntryPtr objects 
    // in that Bucket will also be destructed, if it's the last EntryPtr objects
    // the Entry object will be destructed
    // anyway, this line will make the reference of Entry object decrease one
    connections_.push_back(Bucket());
    dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets() {
    LOG_INFO << "size = " << connections_.size();
    int idx = 0;
    for (auto it = connections_.begin(); it != connections_.end(); ++it, ++idx) {
        // use reference here, or the whole bucekt will be copied
        // if the bucket is big, this would be a time-consuming operation
        Bucket &bucket = *it;
        printf("[%d] len = %zd : ", idx, bucket.size());
        for (auto it2 = bucket.begin(); it2 != bucket.end(); ++it2) {
            bool connectionDead = (*it2)->weakConn_.expired();
            printf("%p(%ld)%s, ", it2->get(), it2->use_count(), connectionDead ? "DEAD" : "");
            //printf("%p(%ld), ", it2->get(), it2->use_count());
        }
        puts("");
    }
}
