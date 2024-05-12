// EPOS Semaphore Implementation

#include <synchronizer.h>

__BEGIN_SYS

Semaphore::Semaphore(long v) : _value(v)
{
    db<Synchronizer>(TRC) << "Semaphore(value=" << _value << ") => " << this << endl;
}


Semaphore::~Semaphore()
{
    db<Synchronizer>(TRC) << "~Semaphore(this=" << this << ")" << endl;
}


void Semaphore::p()
{
    db<Synchronizer>(TRC) << "Semaphore::p(this=" << this << ",value=" << _value << ")" << endl;

    begin_atomic();

    if(fdec(_value) < 1) {
        // Não obtem o semaforo
        // Percorro toda a lista de owners vendo se a thread que tentou um p() tem prioridade maior que os owners
        // for (auto it = _owners.begin(); it != _owners.end(); ++it) {
        //     Thread* owner = it->object();
        //     Thread* current = Thread::self();
        //     // Caso a thread que chegou ter a prioridade maior que a owner
        //     if (current->priority() < owner->priority()) {
        //         int max_priority = current->priority();
        //         // Eu itero sobre a fila waiting buscando se existe alguem que tenha a prioridade maior que a current
        //         for (auto it = _queue.begin(); it != _queue.end(); ++it) {
        //             auto aux = it->object();
        //             if(aux->priority() < current->priority()) {
        //                 // se tem ela se torna a max
        //                 max_priority = aux->priority();
        //             }
        //         }
        //         owner->priority_elevate(max_priority);
        //     }
        // }
        // current eh inserida na fila waiting
        sleep();
    } 
    // else {
    //     // add ao detentor do semaforo
    //     _owners.insert(Thread::self()->link_element());
    // }
        
    end_atomic();
}


void Semaphore::v()
{
    db<Synchronizer>(TRC) << "Semaphore::v(this=" << this << ",value=" << _value << ")" << endl;

    begin_atomic();
    if(finc(_value) < 0) {
        wakeup();
    }
    // Thread* current = Thread::self();
    // current->priority_restore();
    // _owners.remove(current->link_element());
        
    end_atomic();
}

__END_SYS
