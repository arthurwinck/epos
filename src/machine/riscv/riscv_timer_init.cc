// EPOS RISC-V Timer Mediator Initialization

#include <architecture/cpu.h>
#include <machine/timer.h>
#include <machine/ic.h>

__BEGIN_SYS

void Timer::init()
{
    db<Init, Timer>(TRC) << "Timer::init()" << endl;

    assert(CPU::int_disabled());

    // Only initialized once by boostrap processor
    if (CPU::id() == CPU::BSP)
        IC::int_vector(IC::INT_SYS_TIMER, int_handler);

    // We need to synchronize here so timer interruptions are enabled for
    // every core after the int_handler for alarm has been created
    CPU::smp_barrier();

    reset();
    IC::enable(IC::INT_SYS_TIMER);
}

__END_SYS
