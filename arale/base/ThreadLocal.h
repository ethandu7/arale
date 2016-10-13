
#ifndef ARALE_BASE_THREADLOCAL_H
#define ARALE_BASE_THREADLOCAL_H

#include <pthread.h>

namespace arale {

template<typename T>
class ThreadLocal {
public:
    ThreadLocal() {
        pthread_key_create(&key_, &ThreadLocal::destructor);
    }

    ~ThreadLocal() {
        pthread_key_delete(key_);
    }

    T& getValue() {
        T *preObj = pthread_get_specific(key_);
        if (preObj == NULL) {
            T *newObj = new T();
            pthread_set_specific(key_, newObj);
            preObj = newObj;
        }
        return *preObj;
    }
    
private:
    static void destructor(void *x) {
        T *obj = static_cast<T*>(x);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void)dummy;
        delete obj;
    }
    // if it's not a static variable, it should be a global variable
    // the point is that there should be only one key, and every thread should see the same key
    pthread_key key_;
};

}

#endif
