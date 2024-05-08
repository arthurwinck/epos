// EPOS Spin Lock Utility Declarations

#ifndef __spin_h
#define __spin_h

#include <architecture.h>

__BEGIN_UTIL

// Wrapper for the Thread::self() to identifier _owner in Spin
class Thread_Identifier {
public:
    static volatile unsigned long identifier();
};

// Recursive Spin Lock
class Spin
{
public:
    Spin(): _level(0), _owner(0) {}

    void acquire() {
        unsigned long me = Thread_Identifier::identifier();

        while(CPU::cas(_owner, 0UL, me) != me);
        _level++;

        db<Spin>(TRC) << "Spin::acquire[this=" << this << ",id=" << hex << me << "]() => {owner=" << _owner << dec << ",level=" << _level << "}" << endl;
    }

    void release() {
        db<Spin>(TRC) << "Spin::release[this=" << this << "]() => {owner=" << hex << _owner << dec << ",level=" << _level << "}" << endl;

        if(--_level <= 0) {
    	    _level = 0;
            _owner = 0;
    	}
    }

    volatile bool taken() const { return (_owner != 0); }

private:
    volatile long _level;
    volatile unsigned long _owner;
};

// Flat Spin Lock
class Simple_Spin
{
public:
    Simple_Spin(): _locked(false) {}

    void acquire() {
        while(CPU::tsl(_locked));

        db<Spin>(TRC) << "Spin::acquire[SPIN=" << this << "]()" << endl;
    }

    void release() {
        _locked = 0;

        db<Spin>(TRC) << "Spin::release[SPIN=" << this << "]()}" << endl;
    }

private:
    volatile bool _locked;
};

__END_UTIL

#endif
