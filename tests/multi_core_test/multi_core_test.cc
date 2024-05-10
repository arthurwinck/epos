// EPOS Periodic Thread Component Test Program

#include <time.h>
#include <real-time.h>
#include <utility/geometry.h>

using namespace EPOS;

const unsigned int iterations = 100;
const Milisecond period_a = 100;
const Milisecond period_b = 80;
const Milisecond period_c = 60;
const Milisecond wcet_a = 50;
const Milisecond wcet_b = 20;
const Milisecond wcet_c = 10;

int func_a();
int func_b();
int func_c();

OStream cout;
Chronometer chrono;

Periodic_Thread * thread_1;
Periodic_Thread * thread_2;
Periodic_Thread * thread_3;
Periodic_Thread * thread_4;
Periodic_Thread * thread_5;
Periodic_Thread * thread_6;
Periodic_Thread * thread_7;
Periodic_Thread * thread_8;

Point<long, 2> p, p1(2131231, 123123), p2(2, 13123), p3(12312, 123123);

unsigned long base_loop_count;

void callibrate()
{
    chrono.start();
    Microsecond end = chrono.read() + Microsecond(1000000UL);

    base_loop_count = 0;

    while(chrono.read() < end) {
        p = p + Point<long, 2>::trilaterate(p1, 123123, p2, 123123, p3, 123123);
        base_loop_count++;
    }

    chrono.stop();

    base_loop_count /= 1000;
}

inline void exec(char c, Milisecond time = 0)
{
    Milisecond elapsed = chrono.read() / 1000;
    Milisecond end = elapsed + time;
    unsigned int id = Thread::cpu_id();
    Thread * thread_id = Thread::self();

    cout << "\nThread " << thread_id << " on CPU " << id << ": " << elapsed << "func "<< c <<"...";

    while(elapsed < end) {
        for(unsigned long i = 0; i < time; i++)
            for(unsigned long j = 0; j < base_loop_count; j++) {
                p = p + Point<long, 2>::trilaterate(p1, 123123, p2, 123123, p3, 123123);
            }
        elapsed = chrono.read() / 1000;
    }
}



int main()
{
    cout << "Multi cores with Periodic Thread Component Test" << endl;

    cout << "\nThis test consists in creating eight periodic threads" << endl;

    cout << "\nCallibrating the duration of the base execution loop: ";
    callibrate();
    cout << base_loop_count << " iterations per ms!" << endl;

    cout << "\nThreads will now be created and I'll wait for them to finish..." << endl;

    // p,d,c,act,t
    thread_1 = new Periodic_Thread(RTConf(period_a * 1000, 0, wcet_a * 1000, 0, iterations), &func_a);
    thread_2 = new Periodic_Thread(RTConf(period_b * 1000, 0, wcet_b * 1000, 0, iterations), &func_b);
    thread_3 = new Periodic_Thread(RTConf(period_c * 1000, 0, wcet_c * 1000, 0, iterations), &func_c);
    thread_4 = new Periodic_Thread(RTConf(period_a * 1000, 0, wcet_c * 1000, 0, iterations), &func_a);
    thread_5 = new Periodic_Thread(RTConf(period_b * 1000, 0, wcet_c * 1000, 0, iterations), &func_b);
    thread_6 = new Periodic_Thread(RTConf(period_c * 1000, 0, wcet_c * 1000, 0, iterations), &func_c);
    thread_7 = new Periodic_Thread(RTConf(period_a * 1000, 0, wcet_c * 1000, 0, iterations), &func_a);
    thread_8 = new Periodic_Thread(RTConf(period_b * 1000, 0, wcet_c * 1000, 0, iterations), &func_b);

    exec('M');

    chrono.reset();
    chrono.start();

    int status_1 = thread_1->join();
    int status_2 = thread_2->join();
    int status_3 = thread_3->join();
    int status_4 = thread_4->join();
    int status_5 = thread_5->join();
    int status_6 = thread_6->join();
    int status_7 = thread_7->join();
    int status_8 = thread_8->join();

    chrono.stop();

    exec('M');

    cout << "\n... done!" << endl;
    cout << "\n\nThread 1 exited with status \"" << char(status_1)
        << "\", thread 2 exited with status \"" << char(status_2)
        << "\", thread 3 exited with status \"" << char(status_3)
        << "\", thread 4 exited with status \"" << char(status_4)
        << "\", thread 5 exited with status \"" << char(status_5)
        << "\", thread 6 exited with status \"" << char(status_6)
        << "\", thread 7 exited with status \"" << char(status_7)
        << "\" and thread 8 exited with status \"" << char(status_8) << "." << endl;

    cout << "I'm also done, bye!" << endl;

    return 0;
}

int func_a()
{
    exec('A');

    do {
        exec('a', wcet_a);
    } while (Periodic_Thread::wait_next());

    exec('A');

    return 'A';
}

int func_b()
{
    exec('B');

    do {
        exec('b', wcet_b);
    } while (Periodic_Thread::wait_next());

    exec('B');

    return 'B';
}

int func_c()
{
    exec('C');

    do {
        exec('c', wcet_c);
    } while (Periodic_Thread::wait_next());

    exec('C');

    return 'C';
}
