// EPOS Mutex Implementation

#include <synchronizer.h>

__BEGIN_SYS

Mutex::Mutex(): _locked(false)
{
    db<Synchronizer>(TRC) << "Mutex() => " << this << endl;
}


Mutex::~Mutex()
{
    db<Synchronizer>(TRC) << "~Mutex(this=" << this << ")" << endl;
}


void Mutex::lock(LockMode mode)
{
    db<Synchronizer>(TRC) << "Mutex::lock(this=" << this << ")" << endl;

    if (mode == Elevated) {
        begin_atomic(Thread::self());
        if(tsl(_locked))
            sleep();
        end_atomic(Thread::self());
    } else {
        begin_atomic();
        if(tsl(_locked))
            sleep();
        end_atomic();
    }
}

void Mutex::unlock(LockMode mode)
{
    db<Synchronizer>(TRC) << "Mutex::unlock(this=" << this << ")" << endl;

    if (mode == Elevated) {
        begin_atomic(Thread::self());
        if(_queue.empty())
            _locked = false;
        else
            wakeup();
        end_atomic(Thread::self());
    } else {
        begin_atomic();
        if(_queue.empty())
            _locked = false;
        else
            wakeup();
        end_atomic();
    }
}

__END_SYS
