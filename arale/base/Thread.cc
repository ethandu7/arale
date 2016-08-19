
#include <arale/base/CurrentThread.h>
#include <sys/syscall.h>
#include <unistd.h>


namespace arale {

namespace base {

__thread pid_t threadID = 0;
__thread char threadIDStr[32] = { 0 };
__thread int threadIDStr_len = 6;

inline pid_t getCurrentThreadID() {
    if (threadID == 0) {
        threadID = static_cast<pid_t>(::syscall(SYS_gettid));
    }
    return threadID;
}

}

}

