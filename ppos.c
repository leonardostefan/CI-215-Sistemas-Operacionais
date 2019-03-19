#include "ppos.h"
#define STACKSIZE 32768 /* tamanho de pilha das threads */

int lastTaskId;
// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init()
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    lastTaskId = 0;
    setvbuf(stdout, 0, _IONBF, 0);
}

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,               // descritor da nova tarefa
                void (*start_func)(void *), // funcao corpo da tarefa
                void *arg)                  // argumentos para a tarefa
{
    getcontext(&(task->context));
    task->stack = malloc(STACKSIZE);
    if (task->stack)
    {
        task->context.uc_stack.ss_sp = task->stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;
    }
    else
    {
        perror("Erro na criação da pilha: ");
        return (-1);
    }

    ++lastTaskId;
    task->id = lastTaskId;
    makecontext(&(task->context), start_func, arg);


    return lastTaskId;
}
// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exitCode){
    
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
    ucontext_t context;
    getcontext(&context);
    swapcontext(&context,&(task->context));
    return task->id;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id();