
#include <arale/base/CurrentThread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>


namespace arale {

namespace base {

__thread pid_t threadID = 0;
__thread char threadIDStr[32] = { 0 };
__thread int threadIDStr_len = 6;

// do not add inline keyword if you want the function to be seen by others 
pid_t getCurrentThreadID() {
    if (threadID == 0) {
        threadID = static_cast<pid_t>(::syscall(SYS_gettid));
        threadIDStr_len = snprintf(threadIDStr, sizeof(threadIDStr), "%5d", static_cast<int>(threadID));
    }
    return threadID;
}

}

}

