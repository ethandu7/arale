
#ifndef ARALE_BASE_THREADLOCALSINGLETON_H
#define ARALE_BASE_THREADLOCALSINGLETON_H

#include <assert.h>
#include <pthread.h>

namespace arale {

template<typename T>
class ThreadLocalSingleton {
public:
    static T& instance() {
        if (t_value_ == NULL) {
            t_value_ = new T();
            deleter_.set(t_value_);
        }
        return *t_value_;
    }

    static T* pointer() {
        return t_value_;
    }
    
private:
    ThreadLocalSingleton();
    ~ThreadLocalSingleton();

    static void destructor(void *obj) {
        assert(obj == t_value_);
        // you can delete a incomplete type object
        // and at the deletion point, if that incomplete type has a non-trivial destructor
        // it's a undefined behavior, otherwise it's OK

        // can not put sizeof on incomplete type
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy;
        (void)dummy;
        delete t_value_;
        t_value_ = 0;
    }

    // RAII, make sure the T object can be delete automatically
    struct Deleter {
        Deleter() {
            pthread_key_create(&key_, &ThreadLocalSingleton::destructor);
        }

        ~Deleter() {
            pthread_key_delete(key_);
        }

        void set(void *obj) {
            assert(pthread_getspecific(key_) == NULL);
            pthread_setspecific(key_, obj);
        }

        pthread_key_t key_;
    };

    // at least gcc 4.4.7 doesn't support the new C++11 thread_local keyword
    //static thread_local T* t_value_;
    
    static __thread T* t_value_;
    static Deleter deleter_;
};

//template<typename T>
//thread_local T* ThreadLocalSingleton<T>::t_value_ = 0;

template<typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;

// will be created before main, and this Deleter object will be used by all thread
// which can make sure every thread will get the same key 
template<typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;

}

#endif
