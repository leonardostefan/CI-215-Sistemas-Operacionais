/* Código feito pelos alunos Cesar Camillo (GRR20160169) e Leonardo Stefan (GRR20163052) */

#include "ppos.h"
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define STACKSIZE 32768 /* tamanho de pilha das thls \
lsreads */

//Debug Collors
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

//Constantes
#define aging -1
#define QUANTUM_TICKS 20
#define QUANTUM_MICRO_SEG 1000

//Mensagens de erro no debug são printadas com esta função
//Por algum motivo 'inline void' não funciona pra compilar
void messagePrint(char *color, char *erro, char *message)
{
#ifdef DEBUG
    printf("%s%s: %s%s\n", color, erro, RESET, message);
#endif
}

//Definição de variaveis globais
int lastTaskId;
ucontext_t currentContext;
task_t *currentTask, mainTask, dispatcher, *readyTasks, *sleepingTasks, *waitTasks;
struct sigaction action; // estrutura que define o tratador do sinal
struct itimerval timer;  // estrutura de inicialização do timer
int quantum;             // quantum global
unsigned int ticks = 0;
unsigned int initialTime = 0;
unsigned int total = 0;
unsigned int nextWakeup = 0;
//Bloqueio de preempção
char preemption = 1;

task_t *scheduler()
{
    messagePrint(BLU, "scheduler", "realizando scheduling das tarefas");
    task_t *aux = readyTasks->next;
    task_t *task_maior = readyTasks;

    // envelhece as tarefas quando o processo é chamado
    do
    {
        if (aux->id != task_maior->id)
            aux->dina_pri += aging;
        aux = aux->next;
    } while (aux != task_maior);

    // verifica as prioridades para definir a próxima a ser processada
    do
    {
        if (aux->dina_pri < task_maior->dina_pri)
            task_maior = aux;
        aux = aux->next;
    } while (aux != readyTasks);

    task_maior->dina_pri = task_maior->fixed_pri;
    return task_maior;
}
//P9 acordando tarefas adormecidas pelo tempo
int wakeupTasks()
{
    messagePrint(BLU, "task_wakeup", "Acordando tarefas");

    task_t *auxTask = sleepingTasks;
    task_t *removeTask = NULL;
    int qt = 0;
    // int actualTime = systime();
    nextWakeup = 0;
    do
    {
        removeTask = auxTask;
        auxTask = auxTask->next;
        if (removeTask->wakeupTime <= systime())
        {

            queue_remove((queue_t **)&sleepingTasks, (queue_t *)(removeTask));
            queue_append((queue_t **)&readyTasks, (queue_t *)(removeTask));
        }
        else
        {
            if (nextWakeup != 0 || removeTask->wakeupTime < nextWakeup)
            {
                nextWakeup = removeTask->wakeupTime;
            }
        }

    } while (auxTask != NULL && auxTask != sleepingTasks && sleepingTasks != NULL);

    return qt;
}
void dispatcher_body()
{
    task_t *next;
    messagePrint(MAG, "dispatcher", "entrando no dispatcher");
    while ((readyTasks != NULL) || sleepingTasks != NULL)
    {
        if (queue_size((queue_t *)readyTasks) > 0)
        {
            next = scheduler();
            if (next)
            {
                readyTasks = readyTasks->next;

                // ações antes de lançar a tarefa "next", se houverem
                quantum = QUANTUM_TICKS; // ao entrar numa tarefa nova, precisa ser alocado o quantum para o mesmo

                task_switch(next);
                // ações após retornar da tarefa "next", se houverem
            }
        }
        else
        {
            sleep(0);
        }
        //Adicionado para o P9
        if (sleepingTasks != NULL && systime() >= nextWakeup)
        {
            preemption = 0;
            wakeupTasks();
            preemption = 1;
        }
    }
    messagePrint(MAG, "dispatcher", "saindo do dispatcher");
    task_exit(0); // encerra a tarefa no dispatcher
}

void tratador(int signal)
{
    // necessario verificar se é tarefa do usuário ou do sistema
    if (currentTask->id != dispatcher.id)
        if (quantum < 0 && preemption == 1)
            task_yield();

    quantum--;
    ticks++;
}

unsigned int systime()
{
    return ticks++;
}

// inicializa o sistema operacional; deve ser chamada no inicio do main()

void ppos_init()
{

    messagePrint(CYN, "ppos_init", "Inicializando tudo");
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);

    //Criação da task de Main
    mainTask.id = 0;
    lastTaskId = 0;
    mainTask.fixed_pri = 0;
    mainTask.dina_pri = 0;
    getcontext(&(mainTask.context));
    currentTask = &mainTask;
    readyTasks = NULL;
    sleepingTasks = NULL;

    //Inicando Dispatcher
    task_create(&dispatcher, (void *)(*dispatcher_body), "");
    queue_remove((queue_t **)&readyTasks, (queue_t *)&dispatcher);

    //Adicionar Main a fila (P8)
    // if(0)

    //Tratador de ticks
    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("Erro em sigaction: ");
        exit(1);
    }
    timer.it_value.tv_usec = QUANTUM_MICRO_SEG;    // primeiro disparo, em micro-segundos 1000 = 1 millisegundos
    timer.it_value.tv_sec = 0;                     // primeiro disparo, em segundos
    timer.it_interval.tv_usec = QUANTUM_MICRO_SEG; // disparos subsequentes, em micro-segundos 1000 = 1 millisegundos
    timer.it_interval.tv_sec = 0;                  // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("Erro em setitimer: ");
        exit(1);
    }
    initialTime = systime();
    messagePrint(CYN, "ppos_init", "Dispatcher iniciado");

    queue_append((queue_t **)&readyTasks, (queue_t *)&mainTask);
    task_yield();
}

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,               // descritor da nova tarefa
                void (*start_func)(void *), // funcao corpo da tarefa
                void *arg)                  // argumentos para a tarefa
{

    messagePrint(WHT, "task_create", "entrando na função");

    getcontext(&(task->context));

    task->calls = 0;
    task->initialTime = systime();

    task->stack = malloc(STACKSIZE);
    if (task->stack)
    {
        task->context.uc_stack.ss_sp = task->stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
        messagePrint(WHT, "task_create", "pilha criada");
    }
    else
    {
        messagePrint(RED, "task_create", "erro ao criar a pilha");
        perror("Erro na criação da pilha: ");
        return (-1);
    }

    ++lastTaskId;
    task->id = lastTaskId;
    task->fixed_pri = 0;
    task->dina_pri = 0;
    makecontext(&(task->context), (void *)(*start_func), 1, arg);
    // if (task->id != 1)
    queue_append((queue_t **)&readyTasks, (queue_t *)task);
    messagePrint(CYN, "task_create", "Pronto");

    return lastTaskId;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode)
{
    unsigned int exec = systime() - currentTask->initialTime;
    unsigned int process = (currentTask->calls * QUANTUM_TICKS) - quantum;
    unsigned int calls = currentTask->calls;
    //Bloco adicionado no P8
    {

        queue_t *wait_aux;
        while (waitTasks != NULL)
        {
            wait_aux = queue_remove((queue_t **)&waitTasks, (queue_t *)(waitTasks));
            ((task_t *)wait_aux)->joinECode = exitCode;
            queue_append((queue_t **)&readyTasks, wait_aux);
        }
    }
    if (currentTask != &dispatcher)
    {
        queue_remove((queue_t **)&readyTasks, (queue_t *)currentTask);
        total += process;
        printf("Task %d exit:    execution time %u ms,    processor time  %u ms,    %u activations\n", currentTask->id, exec, process, calls);
        task_switch(&dispatcher);
    }
    else
    {
        messagePrint(CYN, "task_exit", "Voltando para a main");
        printf("Task %d exit:    execution time %u ms,    processor time  %u ms,    %u activations\n", currentTask->id, exec, process, calls);

#ifdef DEBUG
        printf("Fim do bagulho, resultado final: %u; deveria ser: %u; ou: %u\n", total, ticks, systime() - initialTime);
#endif
        total += process;

        task_switch(&mainTask);
    }
    messagePrint(CYN, "task_exit", "Tarefa terminada");
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task)
{

    task_t *t = currentTask;
    currentTask = task;
    messagePrint(WHT, "task_switch", "executando troca de contexto");
    (currentTask->calls)++;
    swapcontext(&(t->context), &(currentTask->context));

    return task->id;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id()
{
    return currentTask->id;
}

// libera o processador para a próxima tarefa
void task_yield()
{
    task_switch(&dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio(task_t *task, int prio)
{
    if (task == NULL)
    {
        currentTask->fixed_pri = currentTask->dina_pri = prio;
    }
    else
    {
        task->fixed_pri = task->dina_pri = prio;
    }
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio(task_t *task)
{
    if (task != NULL)
    {
        return task->fixed_pri;
    }
    return currentTask->fixed_pri;
}

//P8
int task_join(task_t *task)
{
    if ((task != NULL) && !((task->next != NULL) ^ (task->prev != NULL)))
    {
        queue_remove((queue_t **)&readyTasks, (queue_t *)currentTask);
        queue_append((queue_t **)&waitTasks, (queue_t *)currentTask);
        task_yield();
        return currentTask->joinECode;
    }
    else
        return -1;
}

//P9
void task_sleep(int t)
{
    currentTask->wakeupTime = systime() + t;
    preemption = 0;
    messagePrint(WHT, "task_sleep", "botando pra dormir");
    queue_remove((queue_t **)&readyTasks, (queue_t *)(currentTask));
    queue_append((queue_t **)&sleepingTasks, (queue_t *)(currentTask));

    if (nextWakeup != 0 || currentTask->wakeupTime < nextWakeup)
    {
        nextWakeup = currentTask->wakeupTime;
    }
    preemption = 1;
    task_yield();
}
//P10
int sem_create(semaphore_t *s, int value)
{   
    s->queue = NULL;
    s->size = value;
    return 0;
}
//P10
int sem_down(semaphore_t *s)
{

    preemption = 0;
    if (s == NULL)
        return -1;
    currentTask->joinECode = 0;

    (s->size)--;
    if (s->size < 0)
    {
        messagePrint(RED, "sem_dow", "manipulando a fila");

        queue_append((queue_t **)&(s->queue), queue_remove((queue_t **)&readyTasks, (queue_t *)currentTask));

        // queue_append((queue_t **)(&(s->queue)), (queue_t *)(currentTask));
        // queue_remove((queue_t **)&readyTasks, (queue_t *)(currentTask));
        task_yield();
    }

    preemption = 1;
    return currentTask->joinECode;
}
//P10

int sem_up(semaphore_t *s)
{
    preemption = 0;
    if (s == NULL)
        return -1;
    s->size++;

    if (s->size <=0 )
    {
        messagePrint(RED, "sem_up", "manipulando a fila");

        queue_append((queue_t **) &readyTasks, queue_remove((queue_t **)&s->queue, (queue_t *) s->queue));

        // queue_t *aux;
        // aux = queue_remove((queue_t **)(&(s->queue)), (queue_t *)(s->queue));
        // queue_append((queue_t **)&readyTasks, (queue_t *)(aux));
    }
    preemption = 1;
    return 0;
}
//P10
int sem_destroy(semaphore_t *s)
{
    if (s == NULL)
        return -1;
    while (s->queue != NULL)
    {
        task_t *aux;
        aux = (task_t *)queue_remove((queue_t **)&(s->queue), (queue_t *)(s->queue));
        aux->joinECode = -1;
        queue_append((queue_t **)&readyTasks, (queue_t *)(aux));
    }
    return 0;
}


// P12
int mqueue_create (mqueue_t *queue, int max, int size){
    if(queue){
        queue->maxSize=max;
        queue->size=size;
        queue->msg=NULL;
        if(sem_create(&queue->vagas, max)==-1 || sem_create(&queue->itens, 0)==-1 ||sem_create(&queue->buffer,1)==-1)
            return -1;
        return 0;
    }
    return -1;
}
// P12
int mqueue_send (mqueue_t *queue, void *msg){
    if(queue && msg){
        if(sem_down(&(queue->vagas))==-1 || sem_down(&(queue->buffer))==-1)
            return (-1);

        filamsg *nova=malloc(sizeof(filamsg));
        nova->msg=malloc(queue->size);
        nova->msg=memcpy (nova->msg, msg, queue->size);
        queue_append((queue_t**)&queue->msg,(queue_t*)nova);

        if( sem_up(&(queue->buffer))==-1 || sem_up(&(queue->itens))==-1)
            return (-1);
        return 0;
    }
    return -1;
}

//P12
int mqueue_recv (mqueue_t *queue, void *msg){
    if(queue){
        if( sem_down(&(queue->itens))==-1 || sem_down(&(queue->buffer))==-1)
            return (-1);
        filamsg *aux = (filamsg*)queue_remove((queue_t**)&queue->msg,(queue_t*)queue->msg);
        msg=memcpy (msg, aux->msg, queue->size);
        free(aux->msg);
        free(aux);

        if(sem_up(&(queue->buffer))==-1 || sem_up(&(queue->vagas))==-1)
            return (-1);
        return 0;
    }
    return -1;
}

//P12
int mqueue_destroy (mqueue_t *queue){
    if(queue){
        while(queue->msg){
            filamsg *aux = (filamsg*)queue_remove((queue_t**)&queue->msg,(queue_t*)queue->msg);
            free(aux->msg);
            free(aux);
        }
        if(sem_destroy (&queue->itens)==-1 || sem_destroy (&queue->vagas)==-1 || sem_destroy (&queue->buffer)==-1)
            return -1;
        return 0;
    }
    return -1;
}