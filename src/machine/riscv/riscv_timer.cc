// EPOS RISC-V Timer Mediator Implementation

#include <machine/ic.h>
#include <machine/timer.h>

__BEGIN_SYS

Timer * Timer::_channels[CHANNELS];

void Timer::int_handler(Interrupt_Id i)
{
    // TODO: There are alarm timer interruptions and scheduler timer interruptions. Why they are treated differently?

    // Only one processor (BSP) handles alarm timer interruptions
    // TODO: Study why this would not happen 
    if((CPU::id() == CPU::BSP) && _channels[ALARM] && (--_channels[ALARM]->_current[CPU::BSP] <= 0)) {
        _channels[ALARM] ->_current[CPU::BSP] = _channels[ALARM]->_initial;
        _channels[ALARM]->_handler(i);
    }

    // Scheduler timers now are handled by the core they are in
    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[CPU::id()] <= 0)) {
        _channels[SCHEDULER]->_current[CPU::id()] = _channels[SCHEDULER]->_initial;
        _channels[SCHEDULER]->_handler(i);
    }
}

__END_SYS