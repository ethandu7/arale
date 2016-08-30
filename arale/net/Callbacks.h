
#ifndef ARALE_NET_CALLBACKS_H
#define ARALE_NET_CALLBACKS_H

#include <arale/base/TimeStamp.h>

#include <functional>
#include <memory>

namespace arale {

namespace net {

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnctionPtr;

typedef std::function<void ()> TimerCallback; 
typedef std::function<void (const TcpConnctionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnctionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnctionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnctionPtr&, size_t)> HighWaterMarkCallback;
typedef std::function<void (const TcpConnctionPtr&, Buffer*, Timestamp)> MessageCallback;

}

}

#endif
