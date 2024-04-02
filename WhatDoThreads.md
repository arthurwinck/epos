// Inicia a thread, aloca a pilha e insere a thread no escalonador.
void Thread::constructor_prologue(unsigned int stack_size);

// Finaliza a configuração da thread, configurando seu estado e, se necessário, fazendo-a elegível para execução imediata.
void Thread::constructor_epilogue(Log_Addr entry, unsigned int stack_size);

// Destrutor da thread, remove a thread do escalonador e libera recursos alocados.
Thread::~Thread();

// Define a prioridade da thread, possivelmente causando uma reordenação no escalonador.
void Thread::priority(const Criterion & c);

// Espera pela conclusão de outra thread.
int Thread::join();

// Passa voluntariamente o controle para a próxima thread pronta a ser executada.
void Thread::pass();

// Suspende a execução da thread atual, removendo-a da fila de execução até que seja explicitamente retomada.
void Thread::suspend();

// Retoma a execução de uma thread previamente suspensa, reinserindo-a na fila de execução.
void Thread::resume();

// Cede a execução para outra thread, permitindo a execução de outras tarefas.
void Thread::yield();

// Termina a execução da thread atual e seleciona a próxima thread a ser executada.
void Thread::exit(int status);

// Coloca a thread atual em espera, baseado em uma condição específica.
void Thread::sleep(Queue \* q);

// Desperta uma única thread que está esperando em uma condição específica.
void Thread::wakeup(Queue \* q);

// Desperta todas as threads que estão esperando em uma condição específica.
void Thread::wakeup_all(Queue \* q);

// Força uma nova escolha de thread a ser executada, baseando-se na prioridade.
void Thread::reschedule();

// Fatia de tempo para a preempção, usada para controlar quanto tempo uma thread pode executar antes de ceder a CPU.
void Thread::time_slicer(IC::Interrupt_Id i);

// Troca o contexto da thread atual pela próxima thread escolhida para execução.
void Thread::dispatch(Thread _ prev, Thread _ next, bool charge);

// Executa enquanto houver mais de uma thread ativa, mantendo o sistema operacional em funcionamento.
int Thread::idle();
