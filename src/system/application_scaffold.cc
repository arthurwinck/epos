// EPOS Application Scaffold and Application Component Implementation

#include <system.h>

// This is a binding not the scaffold -- here so we can test it
extern "C" {
    // Utility methods that differ from kernel and user space.
    // Heap
    static _UTIL::Simple_Spin _heap_spin;
    void _lock_heap() { _heap_spin.acquire(); }
    void _unlock_heap() { _heap_spin.release(); }
}

__BEGIN_SYS

// Application class attributes
char Application::_preheap[];
Heap * Application::_heap;

__END_SYS

__BEGIN_API

// Global objects
__USING_UTIL
OStream cout;
OStream cerr;

__END_API

