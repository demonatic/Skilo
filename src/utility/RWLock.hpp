#ifndef RWLOCK_HPP
#define RWLOCK_HPP

#include <pthread.h>
#include <network/Rinx/include/Util/Noncopyable.h>

namespace Skilo {

using SkiloNoncopyable=Rinx::RxNoncopyable;

class RWLock{
public:
    RWLock(){
        pthread_rwlock_init(&_rw_lock,nullptr);
    }

    ~RWLock(){
        pthread_rwlock_destroy(&_rw_lock);
    }

    void reader_lock(){
        ::pthread_rwlock_rdlock(&_rw_lock);
    }

    void reader_unlock(){
        ::pthread_rwlock_unlock(&_rw_lock);
    }

    void writer_lock(){
        ::pthread_rwlock_wrlock(&_rw_lock);
    }

    void writer_unlock(){
        ::pthread_rwlock_unlock(&_rw_lock);
    }

private:
    pthread_rwlock_t _rw_lock;
};

class WriterLockGuard:public SkiloNoncopyable{
public:
    WriterLockGuard(RWLock &lock):_lock(lock){
        _lock.writer_lock();
    }

    ~WriterLockGuard(){
        _lock.writer_unlock();
    }
private:
    RWLock &_lock;
};

class ReaderLockGuard:public SkiloNoncopyable{
public:
    ReaderLockGuard(RWLock &lock):_lock(lock){
        _lock.reader_lock();
    }

    ~ReaderLockGuard(){
        _lock.reader_unlock();
    }
private:
    RWLock &_lock;
};

} //namespace Skilo

#endif // RWLOCK_HPP
