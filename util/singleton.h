// Copyright (C) 2013
// Author: davidluan
// CreateTime: 2013-02-11


#ifndef __SINGLETON_H__
#define __SINGLETON_H__

//----------------------------------------------------------------------
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

/**
 *	definition of ScopedLock;
 **/
class CScopedLock
{
    friend class condition;

public:
    inline CScopedLock (CMutex& mutex) : _mutex (mutex)
    {
        _mutex.lock ();
    }

    inline ~CScopedLock (void)
    {
        _mutex.unlock ();
    }

private:
    CMutex& _mutex;
};

//----------------------------------------------------------------------
template <class T> struct CreateUsingNew
{
    static T* Create (void)
    {
        return new T;
    }

    static void Destroy(T* p)
    {
        delete p;
    }
};
template <class T, template <class> class CreationPolicy = CreateUsingNew>
class CSingleton
{
public: //method
    static T* Instance (void);
    static void Destroy (void);

public: //properties

protected: //method
protected: //properties

private: //method
    CSingleton (void);
    CSingleton (const CSingleton&);
    CSingleton& operator= (const CSingleton&);

private: //properties
    static T*       _instance;
    static CMutex   _mutex;
};
//----------------------------------------------------------------------

//implement
#include "singleton.inl"
//----------------------------------------------------------------------

#endif //__SINGLETON_H__
