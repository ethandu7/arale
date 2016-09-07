
#ifndef ARALE_BASE_WEAKCALLBACK_H
#define ARALE_BASE_WEAKCALLBACK_H

#include <memory>

namespace arale {

template<typename CLASS, typename... ARGS>
class WeakCallback {
public:
    WeakCallback(const std::weak_ptr<CLASS> &obj, 
                 const std::function<void (CLASS*, ARGS...)> &function) 
        : obj_(obj), function_(function) 
   {
   }

   void operator()(ARGS&&... args) {
       std::shared_ptr<CLASS> ptr(obj_.lock());
       if (ptr) {
            function_(ptr.get(), std::forward<ARGS>(args)...);
       }
   }
private:
    std::weak_ptr<CLASS> obj_;
    std::function<void (CLASS*, ARGS...)> function_;
};

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWekaCallback(const std::shared_ptr<CLASS> &obj,
                                            void (CLASS::*function)(ARGS...)) {
    return WeakCallback<CLASS, ARGS...>(obj, function);
}

}

#endif
