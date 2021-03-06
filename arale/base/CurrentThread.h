
#ifndef ARALE_NET_CURRENTTHREAD_H
#define ARALE_NET_CURRENTTHREAD_H
#include <sys/types.h>


namespace arale {

namespace base {

extern __thread pid_t threadID;
// this two are for logging
extern __thread char threadIDStr[32];
extern __thread int threadIDStr_len;

pid_t getCurrentThreadID();


// don't forget inline keyword, otherwise it will get "multiple definition" error
inline const char* getThreadIDStr() {
    return threadIDStr;
}

inline int getThreadIDStrLen() {
    return threadIDStr_len;
}

}


}


#endif
