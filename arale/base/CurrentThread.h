
#ifndef ARALE_NET_CURRENTTHREAD_H
#define ARALE_NET_CURRENTTHREAD_H

#include <pthread>


namespace arale {

namespace base {

extern __thread int threadID;

int getCurrentThreadID();

};


};


#endif
