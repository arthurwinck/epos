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


void Mutex::lock()
{
    db<Synchronizer>(TRC) << "Mutex::lock(this=" << this << ")" << endl;

    begin_atomic();
    if(tsl(_locked)) {
        // Thread* _owner = _owners.head()->object();
        // Thread* current = Thread::self();
        // if(current->priority() < _owner->priority()) {
        //     int max_priority = current->priority();
        //     // Eu itero sobre a fila waiting buscando se existe alguem que tenha a prioridade maior que a current
        //     for (auto it = _queue.begin(); it != _queue.end(); ++it) {
        //         auto aux = it->object();
        //         if(aux->priority() < current->priority()) {
        //             // se tem ela se torna a max
        //             max_priority = aux->priority();
        //         }
        //     }
        //     _owner->priority_elevate(max_priority);
        // }
        sleep();
    } 
    // else {
    //     _owners.insert(Thread::self()->link_element());
    // }
        
    end_atomic();
}


void Mutex::unlock()
{
    db<Synchronizer>(TRC) << "Mutex::unlock(this=" << this << ")" << endl;

    begin_atomic();
    if(_queue.empty()) {
        _locked = false;
        
    } else {
        wakeup();
    }
    // Thread* current = Thread::self();
    // current->priority_restore();
    // _owners.remove(current->link_element());
    end_atomic();
}

__END_SYS
