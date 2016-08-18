
#include <arale/base/CurrentThread.h>
#include <sys/syscall.h>
#include <unistd.h>


namespace arale {

namespace base {

__thread pid_t threadID = 0;

inline pid_t getCurrentThreadID() {
    if (threadID == 0) {
        threadID = syscall(SYS_gettid);
    }
    return threadID;
}

};

};

