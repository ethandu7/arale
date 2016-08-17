
#include <arale/base/CurrentThread.h>
#include <sys/syscall.h>
#include <unistd.h>


using namespace arale;
using namespace arale::base;

__thread int arale::base::threadID = 0;

inline int getCurrentThreadID() {
    if (threadID == 0) {
        threadID = syscall(SYS_gettid);
    }
    return threadID;
}

