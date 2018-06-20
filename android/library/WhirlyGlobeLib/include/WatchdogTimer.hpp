//
//  WatchdogTimer.hpp
//  WhirlyGlobeLib
//
//  Created by Alex St George on 2018-06-14.
//

#include <pthread.h>
#include <functional>

namespace WhirlyKit
{
    class WatchdogTimer
    {
    public:
        WatchdogTimer();
        WatchdogTimer(unsigned int milliseconds, std::function<void()> callback);
        ~WatchdogTimer();
        
        void start();
        void stop();
        void reset();
        
        const bool isValid();
        const bool isRunning();
        
    private:
        static void* _startThreadFunction(void* timer_ptr);
        void _threadFunction();
        
        pthread_t _thread;
        pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t  _condition = PTHREAD_COND_INITIALIZER;
        std::function<void()> _callback;
        
        unsigned int _secInterval;
        unsigned long _nsecInterval;
        
        bool _valid;
        bool _running;
    };
}
