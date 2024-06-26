// EPOS Initializer End

#include <architecture.h>
#include <system.h>
#include <process.h>

__BEGIN_SYS

// This class purpose is simply to define a well-known ending point for the initialization of the system.
// It activates the first application thread (usually main()).
// It must be linked first so init_end becomes the last constructor in the global's constructor list.

class Init_End
{
public:
    Init_End() {
        db<Init>(TRC) << "Init_End()" << endl;
        
        // We need to synchronize the creation of the main thread of all the cores
        CPU::smp_barrier();

        if(!Traits<System>::multithread) {
            CPU::int_enable();
            return;
        }

        // Since we still need to free up the boot stack used, lets keep it here
        if(CPU::id() == CPU::BSP && Memory_Map::BOOT_STACK != Memory_Map::NOT_USED)
            MMU::free(Memory_Map::BOOT_STACK, MMU::pages(Traits<Machine>::STACK_SIZE));

        db<Init>(INF) << "INIT ends here!" << endl;

        CPU::smp_barrier();

        if (Traits<System>::multicore) {
            // Enabling IPIs
            IC::int_vector(IC::INT_RESCHEDULER, Thread::rescheduler);  // Handler for rescheduler is just locking and sending a reschedule to that core  
            IC::enable(IC::INT_RESCHEDULER);                           // Enabling int_rescheduler is enabling software interrupts
        }

        // Thread::self() and Task::self() can be safely called after the construction of MAIN
        // even if no reschedule() was called (running is set by the Scheduler at each insert())
        // It will return MAIN for CPU0 and IDLE for the others
        Thread * first = Thread::self();

        db<Init, Thread>(INF) << "Dispatching the first thread: " << first << endl;
        // Interrupts have been disabled at Thread::init() and will be reenabled by CPU::Context::load()
        // but we first reset the timer to avoid getting a time interrupt during load()
        if(Traits<Timer>::enabled)
            Timer::reset();

        db<Init>(WRN) << "My mstatus is: " << CPU::status() << endl;

        first->_context->load();
    }
};

Init_End init_end;

__END_SYS
