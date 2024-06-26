// EPOS Scheduler Component Declarations

#ifndef __scheduler_h
#define __scheduler_h

#include <architecture/cpu.h>
#include <architecture/pmu.h>
#include <architecture/tsc.h>
#include <utility/scheduling.h>
#include <utility/math.h>
#include <utility/convert.h>

__BEGIN_SYS

// All scheduling criteria, or disciplines, must define operator int() with
// the semantics of returning the desired order of a given object within the
// scheduling list
class Scheduling_Criterion_Common
{
    friend class _SYS::Thread;
    friend class _SYS::Periodic_Thread;
    friend class _SYS::RT_Thread;
    friend class _SYS::Clerk<System>;         // for _statistics

protected:
    typedef Timer_Common::Tick Tick;
public:
    // Priorities
    enum : int {
        CEILING = -1000,
        MAIN    = -1,
        HIGH    = 0,
        NORMAL  = (unsigned(1) << (sizeof(int) * 8 - 3)) - 1,
        LOW     = (unsigned(1) << (sizeof(int) * 8 - 2)) - 1,
        IDLE    = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
    };

    // Constructor helpers
    enum : unsigned int {
        SAME        = 0,
        NOW         = 0,
        UNKNOWN     = 0,
        ANY         = -1U
    };

    // Policy types
    enum : int {
        PERIODIC    = HIGH,
        APERIODIC   = NORMAL,
        SPORADIC    = NORMAL
    };

    // Policy events
    typedef int Event;
    enum {
        CREATE          = 1 << 0,
        FINISH          = 1 << 1,
        ENTER           = 1 << 2,
        LEAVE           = 1 << 3,
        JOB_RELEASE     = 1 << 4,
        JOB_FINISH      = 1 << 5
    };

    // Policy operations
    typedef int Operation;
    enum {
        COLLECT         = 1 << 16,
        CHARGE          = 1 << 17,
        AWARD           = 1 << 18,
        UPDATE          = 1 << 19
    };

    // Policy traits
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool collecting = false;
    static const bool charging = false;
    static const bool awarding = false;
    static const bool migrating = false;
    static const bool track_idle = false;
    static const bool task_wide = false;
    static const bool cpu_wide = false;
    static const bool system_wide = false;

    //default values
    static const unsigned int QUEUES = 1;
    static const unsigned int HEADS = Traits<System>::CPUS;
    
    // Runtime Statistics (for policies that don't use any; that's why its a union)
    union Statistics {
        // Thread related statistics
        Tick thread_creation;                   // tick in which the thread was created
        Tick thread_destruction;                // tick in which the thread was destroyed
        Tick thread_execution_time;             // accumulated execution time (in ticks)
        Tick thread_last_dispatch;              // tick in which the thread was last dispatched to the CPU
        Tick thread_last_dispatch_on_core[Traits<System>::CPUS];
        Tick thread_last_preemption;            // tick in which the thread left the CPU by the last time

        // Job related statistics
        bool job_released;
        Tick job_release;                       // tick in which the last job of a periodic thread was made ready for execution
        Tick job_start;                         // tick in which the last job of a periodic thread started (different from "thread_last_dispatch" since jobs can be preempted)
        Tick job_finish;                        // tick in which the last job of a periodic thread finished (i.e. called _alarm->p() at wait_netxt(); different from "thread_last_preemption" since jobs can be preempted)
        Tick job_utilization;                   // accumulated execution time (in ticks)
        unsigned int jobs_released;             // number of jobs of a thread that were released so far (i.e. the number of times _alarm->v() was called by the Alarm::handler())
        unsigned int jobs_finished;             // number of jobs of a thread that finished execution so far (i.e. the number of times alarm->p() was called at wait_next())

        // CPU related statistics
        Tick execution_per_cpu[Traits<System>::CPUS]; // Accumulated execution time PER CPU
        
    };

protected:
    Scheduling_Criterion_Common(): _statistics() {}

public:
    void period(const Microsecond & p) {}

    bool update() { return false; }
    bool update_capacity() { return false; }

    bool collect(Event event) { return false; }
    bool charge(bool end = false) { return true; }
    bool award(bool end = false) { return true; }

    bool periodic() { return false; }

    volatile Statistics & statistics() { return _statistics; }

    static void init() {}

    // Default values for Global scheduler implementations
    static unsigned int current_head() { return CPU::id(); }
    static unsigned int current_queue() { return 0; }

protected:
    Statistics _statistics;
};

// Tentamos implementar essa logica diretamente no Priority, porem por algum motivo as queues não ficavam de forma circular
// Ultilizamos a solucao do EPOS referencia para implementar a logica de queues
class Variable_Queue_Scheduler
{
protected:
    Variable_Queue_Scheduler(unsigned int queue): _queue(queue) {};

    const volatile unsigned int & queue() const volatile { return _queue; }
    void queue(unsigned int q) { _queue = q; }

protected:
    volatile unsigned int _queue;
    static volatile unsigned int _next_queue;
};

// Priority (static and dynamic)
class Priority: public Scheduling_Criterion_Common
{
    friend class _SYS::Thread;
    friend class _SYS::Periodic_Thread;
    friend class _SYS::RT_Thread;

public:
    template <typename ... Tn>
    Priority(int p = NORMAL, Tn & ... an): _priority(p) { _queue = 0; }

    const volatile unsigned int & queue() const volatile { return _queue; }

    operator const volatile int() const volatile { return _priority; }

protected:
    volatile int _priority;

private:
    volatile unsigned int _queue;
};

// Round-Robin
class RR: public Priority
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool partitioned = false;

public:
    template <typename ... Tn>
    RR(int p = NORMAL, Tn & ... an): Priority(p) {}
};

// First-Come, First-Served (FIFO)
class FCFS: public Priority
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = false;
    static const bool partitioned = false;

public:
    template <typename ... Tn>
    FCFS(int p = NORMAL, Tn & ... an);
};


// Real-time Algorithms
class Real_Time_Scheduler_Common: public Priority
{
protected:
    Real_Time_Scheduler_Common(int i): Priority(i), _period(0), _deadline(0), _capacity(0) {} // aperiodic
    Real_Time_Scheduler_Common(int i, const Microsecond & d, const Microsecond & p, const Microsecond & c)
    : Priority(i), _period(ticks(p)), _deadline(ticks(d ? d : p)), _capacity(ticks(c)) {}

public:
    bool periodic() { return (_priority >= PERIODIC) && (_priority <= SPORADIC); }

    void collect(Event event);

    Microsecond period() { return time(_period); }
    Microsecond deadline() { return time(_deadline); }
    Microsecond capacity() { return time(_capacity); }

    volatile Statistics & statistics() { return _statistics; }

protected:
    static Tick elapsed();
    Tick ticks(Microsecond time);
    Microsecond time(Tick ticks);

    Tick _period;
    Tick _deadline;
    Tick _capacity;
};

// Rate Monotonic
class RM: public Real_Time_Scheduler_Common
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool partitioned = false;

public:
    RM(int p = APERIODIC): Real_Time_Scheduler_Common(p) {}
    RM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : Real_Time_Scheduler_Common(int(ticks(p)), p, d, c) {}
};

// Deadline Monotonic
class DM: public Real_Time_Scheduler_Common
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool partitioned = false;

public:
    DM(int p = APERIODIC): Real_Time_Scheduler_Common(p) {}
    DM(const Microsecond & d, const Microsecond & p = SAME, const Microsecond & c = UNKNOWN, unsigned int cpu = ANY)
    : Real_Time_Scheduler_Common(int(ticks(d ? d : p)), p, d, c) {}
};

// Earliest Deadline First
class EDF: public Real_Time_Scheduler_Common
{
public:
    static const bool timed = true;
    static const bool dynamic = true;
    static const bool preemptive = true;
    static const bool partitioned = false;

public:
    EDF(int p = APERIODIC): Real_Time_Scheduler_Common(p) {}
    EDF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN);

    void update();
};


// Least Laxity First
class LLF: public Real_Time_Scheduler_Common
{
public:
    static const bool timed = true;
    static const bool dynamic = true;
    static const bool preemptive = true;
    static const bool partitioned = false;
    

public:
    LLF(int p = APERIODIC): Real_Time_Scheduler_Common(p) {}
    LLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN);

    void update();
};

// Global Least Laxity First
class GLLF: public LLF
{
public:
    static const bool partitioned = false;

public:
    GLLF(int p = APERIODIC): LLF(p) {}
    GLLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN) : LLF(p, d, c) {}

    static unsigned int current_head() { return CPU::id(); }

    static unsigned int current_queue() { return 0; }

    static const unsigned int QUEUES = 1;
    static const unsigned int HEADS = Traits<System>::CPUS;
};

// Partitioned Least Laxity First
class PLLF: public LLF, public Variable_Queue_Scheduler
{
public:
    static const bool partitioned = true;

public:
    PLLF(int p = APERIODIC): LLF(p), Variable_Queue_Scheduler(
        ((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : ++_next_queue %= CPU::cores()
    ) {}
    PLLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN) : LLF(p, d, c), Variable_Queue_Scheduler(
        ((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : ++_next_queue %= CPU::cores()
    ) {}

    static unsigned int current_head() { return 0; }

    static unsigned int current_queue() { return CPU::id(); }

    using Variable_Queue_Scheduler::queue;

    static const unsigned int QUEUES = Traits<System>::CPUS;
    static const unsigned int HEADS = 1;
};

__END_SYS

#endif
