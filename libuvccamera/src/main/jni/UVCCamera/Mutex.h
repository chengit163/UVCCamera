#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

class Mutex
{
private:
    pthread_mutex_t m_mutex;

public:
    Mutex();
    virtual ~Mutex();

private:
    Mutex(const Mutex &) {};
    Mutex &operator=(const Mutex &) {};

public:
    int lock();
    int unlock();
    int trylock();
};

#endif //__MUTEX_H__