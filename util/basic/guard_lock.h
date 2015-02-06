/*****************************************
* added by vincenthuang
* 2009-10-09
******************************************/

#ifndef __GUARD_LOCK_H__
#define __GUARD_LOCK_H__

#include <pthread.h>

class CMutex
{
    friend class condition;

public:
    inline CMutex (void)
    {
        ::pthread_mutex_init (&_mutex, 0);
    }

    inline void lock (void)
    {
        ::pthread_mutex_lock (&_mutex);
    }

    inline void unlock (void)
    {
        ::pthread_mutex_unlock (&_mutex);
    }

    inline ~CMutex (void)
    {
        ::pthread_mutex_destroy (&_mutex);
    }

private:
    CMutex (const CMutex& m);
    const CMutex& operator= (const CMutex &m);

private:
    pthread_mutex_t _mutex;
};

static CMutex mutex_;

class CGuardLock
{
	friend class condition;

public:
    inline CGuardLock (CMutex& mutex) : _mutex (mutex)
    {
        _mutex.lock ();
    }

    inline ~CGuardLock (void)
    {
        _mutex.unlock ();
    }

private:
    CMutex& _mutex;
};

#endif
