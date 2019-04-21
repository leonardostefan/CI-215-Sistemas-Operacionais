/* Código feito pelos alunos Cesar Camillo (GRR20160169) e Leonardo Stefan (GRR20163052) */

#include "ppos.h"
#include "stdlib.h"
#include <stdio.h>
#include "queue.h"
#include <signal.h>
#include <sys/time.h>

#define STACKSIZE 32768 /* tamanho de pilha das threads */

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

int lastTaskId;
ucontext_t currentContext;
task_t *currentTask, mainTask, dispatcher, *userTasks;
struct sigaction action; // estrutura que define o tratador do sinal
struct itimerval timer; // estrutura de inicialização do timer
int quantum; // quantum global

task_t *scheduler()
{
    messagePrint(BLU, "scheduler", "realizando scheduling das tarefas");
    task_t *aux = userTasks->next;
    task_t *task_maior = userTasks;

    // envelhece as tarefas quando o processo é chamado
    do{
        if (aux->id != task_maior->id)
            aux->dina_pri += aging;
        aux = aux->next;
    }while (aux != task_maior);

    // verifica as prioridades para definir a próxima a ser processada
    do{
        if (aux->dina_pri < task_maior->dina_pri)
            task_maior = aux;
        aux = aux->next;
    }while (aux != userTasks);

    task_maior->dina_pri = task_maior->fixed_pri;
    return task_maior;   
}

void dispatcher_body ()
{     
    task_t *next;
    messagePrint(MAG, "dispatcher", "entrando no dispatcher");
    while (queue_size((queue_t*)userTasks) > 0 ){
      next = scheduler() ;
      if (next){
        userTasks = userTasks->next; 

        // ações antes de lançar a tarefa "next", se houverem
        quantum = QUANTUM_TICKS; // ao entrar numa tarefa nova, precisa ser alocado o quantum para o mesmo

        task_switch (next) ; 
        // ações após retornar da tarefa "next", se houverem
      }
    }
    messagePrint(MAG, "dispatcher", "saindo do dispatcher");
    task_exit(0) ; // encerra a tarefa no dispatcher
}

void tratador (int signal)
{
	// necessario verificar se é tarefa do usuário ou do sistema
	if (currentTask->id != dispatcher.id)
		if (quantum == 0)
			task_yield();

	quantum--;
}

// inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init()
{
    messagePrint(CYN, "ppos_init", "Inicializando tudo");
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
    mainTask.id = 0;
    lastTaskId = 0;
    mainTask.fixed_pri = 0;
    mainTask.dina_pri = 0;
    getcontext(&(mainTask.context));
    currentTask = &mainTask;
    userTasks = NULL;
    task_create(&dispatcher, (void *)(*dispatcher_body),"");
    queue_remove((queue_t**)&userTasks, (queue_t*)&dispatcher);

    action.sa_handler = tratador;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }
    timer.it_value.tv_usec = QUANTUM_MICRO_SEG;      // primeiro disparo, em micro-segundos 1000 = 1 millisegundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = QUANTUM_MICRO_SEG;   // disparos subsequentes, em micro-segundos 1000 = 1 millisegundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos
    
    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
       perror ("Erro em setitimer: ") ;
       exit (1) ;
    }

    messagePrint(CYN, "ppos_init", "Dispatcher iniciado");
}

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,               // descritor da nova tarefa
                void (*start_func)(void *), // funcao corpo da tarefa
                void *arg)                  // argumentos para a tarefa
{

    messagePrint(WHT, "task_create", "entrando na função");

    getcontext(&(task->context));
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
    if(task->id != 1 )
        queue_append((queue_t**)&userTasks,(queue_t*)task);
    messagePrint(CYN, "task_create", "Pronto");

    return lastTaskId;}


// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode)
{
    if(currentTask != &dispatcher){
        queue_remove((queue_t**)&userTasks,(queue_t*)currentTask); 
        task_switch(&dispatcher);
    }
    else{
        messagePrint(CYN, "task_exit", "Voltando para a main");
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
    swapcontext(&(t->context),&(currentTask->context));

    return task->id;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id(){
    return currentTask->id;
}

// libera o processador para a próxima tarefa
void task_yield (){
    task_switch(&dispatcher);    
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio){
    if (task == NULL){
        currentTask->fixed_pri = currentTask->dina_pri = prio;
    }
    else{
        task->fixed_pri = task->dina_pri = prio;
    }
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task){
    if (task != NULL){
        return task->fixed_pri;
    }
    return currentTask->fixed_pri;
}