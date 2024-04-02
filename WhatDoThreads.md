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

Métodos a Serem Adaptados
reschedule() e time_slicer(): Estes são os principais pontos onde o escalonamento toma uma decisão sobre qual thread deve executar a seguir. Antes de escolher a próxima thread a ser executada, você deve assegurar que as prioridades estejam atualizadas para refletir as demandas atuais. Portanto, chamar um método de atualização de prioridades aqui garante que a escolha da próxima thread a ser executada considere as prioridades mais recentes.

sleep() e wakeup()/wakeup_all(): Quando uma thread é posta em espera ou despertada, isso pode alterar significativamente a sua urgência ou deadlines. Integrar a atualização de prioridades após essas ações pode ajudar a manter o escalonador alinhado com as necessidades reais das threads.

Thread::constructor_epilogue(): Ao finalizar a criação de uma thread, estabelecer sua prioridade inicial com base na lógica de LLF pode ser uma boa prática, especialmente para sistemas que necessitam de um escalonamento sensível ao tempo desde o início.

Implementação da Atualização de Prioridade
Integrar Atualizações de Prioridade: Em reschedule() e time_slicer(), antes de escolher a próxima thread, invoque um método, por exemplo, update_all_priorities(), que percorra todas as threads aptas e atualize suas prioridades com base na laxidade ou slack time atual.

void Thread::reschedule() {
lock();
update*all_priorities(); // Atualiza a prioridade de todas as threads
Thread * prev = running();
Thread \_ next = \_scheduler.choose();
dispatch(prev, next);
unlock();
}

Atualização de Prioridade em Eventos de Sono e Despertar: Em sleep() e wakeup()/wakeup_all(), considere chamar update_priority() para a thread em questão, ajustando sua prioridade individualmente conforme seu estado muda.

void Thread::wakeup(Queue _ q) {
lock();
if(!q->empty()) {
Thread _ t = q->remove()->object();
t->update_priority(); // Atualiza a prioridade da thread específica
t->\_state = READY;
\_scheduler.resume(t);
}
if(preemptive) reschedule();
unlock();
}
