//
//  WatchdogTimer.cpp
//  WhirlyGlobeLib
//
//  Created by Alex St George on 2018-06-14.
//

#include <system_error>
#include "WatchdogTimer.hpp"

using namespace WhirlyKit;

namespace WhirlyKit {
    WatchdogTimer::WatchdogTimer(): _valid(false), _running(false), _secInterval(0), _nsecInterval(0), _callback({}){}
    
    WatchdogTimer::WatchdogTimer(unsigned int milliseconds, std::function<void()> callback)
    : _valid(true), _running(false)
    {
        _secInterval = milliseconds/1000;
        _nsecInterval = (milliseconds - _secInterval * 1000) * 1000000;
        _callback = callback;
    }
    
    WatchdogTimer::~WatchdogTimer()
    {
        stop();
        pthread_mutex_destroy(&_lock);
        pthread_cond_destroy(&_condition);
    }
    
    void WatchdogTimer::start()
    {
        pthread_mutex_lock(&_lock);
        if (!_running && _valid) {
            _running = true;
            pthread_create(&_thread, NULL, WatchdogTimer::_startThreadFunction, this);
        }
        pthread_mutex_unlock(&_lock);
    }
    
    void WatchdogTimer::stop()
    {
        pthread_mutex_lock(&_lock);
        if (_running) {
            _valid = false;
            pthread_mutex_unlock(&_lock);
            pthread_cond_signal(&_condition);
            pthread_join(_thread, NULL);
            return;
        }
        pthread_mutex_unlock(&_lock);
    }
    
    void WatchdogTimer::reset()
    {
        pthread_mutex_lock(&_lock);
        if (_running && _valid) {
            pthread_mutex_unlock(&_lock);
            pthread_cond_signal(&_condition);
            return;
        }
        pthread_mutex_unlock(&_lock);
    }
    
    const bool WatchdogTimer::isValid()
    {
        bool value;
        pthread_mutex_lock(&_lock);
        value = _valid;
        pthread_mutex_unlock(&_lock);
        return value;
    }
    
    const bool WatchdogTimer::isRunning()
    {
        bool value;
        pthread_mutex_lock(&_lock);
        value = _running;
        pthread_mutex_unlock(&_lock);
        return value;
    }
    void* WatchdogTimer::_startThreadFunction(void* timer_ptr)
    {
        if (timer_ptr)
            static_cast<WatchdogTimer*>(timer_ptr)->_threadFunction();
        pthread_exit(NULL);
    }
    
    void WatchdogTimer::_threadFunction()
    {
        int flag;
        struct timespec timeoutTime;
        pthread_mutex_lock(&_lock);
        
        while (_valid) {
            clock_gettime(CLOCK_REALTIME, &timeoutTime);
            timeoutTime.tv_sec += _secInterval;
            timeoutTime.tv_nsec += _nsecInterval;
            
            if (timeoutTime.tv_nsec > 999999999) {
                timeoutTime.tv_nsec += 1;
                timeoutTime.tv_nsec = timeoutTime.tv_nsec % 1000000000;
            }
            
            flag = pthread_cond_timedwait(&_condition, &_lock, &timeoutTime);
            
            if (flag == ETIMEDOUT) {
                _valid = false;
                _running = false;
                pthread_mutex_unlock(&_lock);
                _callback();
                pthread_exit(NULL);
            }
        }
        _running = false;
        pthread_mutex_unlock(&_lock);
        pthread_exit(NULL);
    }
}
