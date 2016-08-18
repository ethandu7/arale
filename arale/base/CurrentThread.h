
#ifndef ARALE_NET_CURRENTTHREAD_H
#define ARALE_NET_CURRENTTHREAD_H
#include <sys/types.h>


namespace arale {

namespace base {

extern __thread pid_t threadID;

pid_t getCurrentThreadID();

};


};


#endif
