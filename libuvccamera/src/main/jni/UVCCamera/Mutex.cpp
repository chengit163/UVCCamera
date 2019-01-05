#include "Mutex.h"

Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

int Mutex::lock()
{
    return pthread_mutex_lock(&m_mutex);
}

int Mutex::unlock()
{
    return pthread_mutex_unlock(&m_mutex);
}

int Mutex::trylock()
{
    return pthread_mutex_trylock(&m_mutex);
}