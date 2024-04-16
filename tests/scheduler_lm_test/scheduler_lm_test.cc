// EPOS Scheduler Test Program

#include <machine/display.h>
#include <synchronizer.h>
#include <process.h>
#include <real-time.h>

using namespace EPOS;

const int iterations = 10;

Mutex table;

Periodic_Thread * phil[5];
Semaphore * chopstick[5];

OStream cout;

int philosopher(int n, int l, int c);

int philosopher_0() { return philosopher(0, 5, 32); }
int philosopher_1() { return philosopher(1, 10, 44); }
int philosopher_2() { return philosopher(2, 16, 39); }
int philosopher_3() { return philosopher(3, 16, 24); }
int philosopher_4() { return philosopher(4, 10, 20); }

int main()
{
    table.lock();
    Display::clear();
    Display::position(0, 0);
    cout << "LM TEST" << endl;
    cout << "The Philosopher's Dinner:" << endl;

    for(int i = 0; i < 5; i++)
        chopstick[i] = new Semaphore;

    const int period_base = 1000; // Base para calcular o período em microssegundos
    const int iterations = 10;

    // Definições de cada philosopher usando Periodic_Thread
    phil[0] = new Periodic_Thread(RTConf(period_base * 1.0, period_base * 0.9, period_base * 0.5, 0, iterations), &philosopher_0);
    phil[1] = new Periodic_Thread(RTConf(period_base * 1.2, period_base * 1.2 * 0.9, period_base * 1.2 * 0.5, 0, iterations), &philosopher_1);
    phil[2] = new Periodic_Thread(RTConf(period_base * 1.4, period_base * 1.4 * 0.9, period_base * 1.4 * 0.5, 0, iterations), &philosopher_2);
    phil[3] = new Periodic_Thread(RTConf(period_base * 1.6, period_base * 1.6 * 0.9, period_base * 1.6 * 0.5, 0, iterations), &philosopher_3);
    phil[4] = new Periodic_Thread(RTConf(period_base * 1.8, period_base * 1.8 * 0.9, period_base * 1.8 * 0.5, 0, iterations), &philosopher_4);
    cout << "Philosophers are alive and hungry!" << endl;

    Display::position(7, 44);
    cout << '/';
    Display::position(13, 44);
    cout << '\\';
    Display::position(16, 35);
    cout << '|';
    Display::position(13, 27);
    cout << '/';
    Display::position(7, 27);
    cout << '\\';
    Display::position(19, 0);

    cout << "The dinner is served ..." << endl;
    table.unlock();

    for(int i = 0; i < 5; i++) {
        int ret = phil[i]->join();
        table.lock();
        Display::position(20 + i, 0);
        cout << "Philosopher " << i << " ate " << ret << " times " << endl;
        table.unlock();
    }

    for(int i = 0; i < 5; i++)
        delete chopstick[i];
    for(int i = 0; i < 5; i++)
        delete phil[i];

    cout << "The end!" << endl;

    return 0;
}

int philosopher(int n, int l, int c)
{
    int first = (n < 4)? n : 0;
    int second = (n < 4)? n + 1 : 4;

    for(int i = iterations; i > 0; i--) {

        table.lock();
        Display::position(l, c);
        cout << "thinking";
        table.unlock();

        Delay thinking(1000000);

        table.lock();
        Display::position(l, c);
        cout << " hungry ";
        table.unlock();

        chopstick[first]->p();   // get first chopstick
        chopstick[second]->p();  // get second chopstick

        table.lock();
        Display::position(l, c);
        cout << " eating ";
        table.unlock();

        Delay eating(500000);

        table.lock();
        Display::position(l, c);
        cout << "  sate  ";
        table.unlock();

        chopstick[first]->v();   // release first chopstick
        chopstick[second]->v();  // release second chopstick
    }

    table.lock();
    Display::position(l, c);
    cout << "  done  ";
    table.unlock();

    return iterations;
}
