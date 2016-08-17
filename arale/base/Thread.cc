
#include <arale/base/CurrentThread.h>
#include <sys/syscall.h>

using namespace muduo;
using namespace muduo::base;

__thread int threadID = 0;

inline int getCurrentThreadID() {
    if (threadID == 0) {
        threadID = ::gettid();
    }
    return threadID;
}

