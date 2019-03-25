#include "ppos.h"
#include "stdlib.h"
#include <stdio.h>

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
task_t *currentTask, *mainTask;
// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);
    mainTask = calloc(1, sizeof(task_t));
    mainTask->id = 0;
    lastTaskId = 0;
    currentTask = mainTask;
    getcontext(&(mainTask->context));

    messagePrint(CYN, "ppos_init", "S.O. iniciado");
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
    makecontext(&(task->context), (void *)(*start_func), 1, arg);

    messagePrint(CYN, "task_create", "Pronto");

    return lastTaskId;
}
// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode)
{
    ucontext_t context;
    getcontext(&context);

    swapcontext(&context, &(mainTask->context));
    messagePrint(CYN, "task_exit", "Tarefa terminada");
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task)
{

    task_t *t = currentTask;
    currentTask = task;
    messagePrint(WHT, "task_switch", "executando troca de contexto");
    swapcontext(&(t->context),&( currentTask->context));

    return task->id;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id(){
    return currentTask->id;
}