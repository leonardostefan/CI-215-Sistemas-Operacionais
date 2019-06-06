// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"		// biblioteca de filas genéricas

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
   struct task_t *prev, *next ;		// ponteiros para usar em filas
   int id ;				// identificador da tarefa
   ucontext_t context ;			// contexto armazenado da tarefa
   void *stack ;			// aponta para a pilha da tarefa
   int fixed_pri; 		// por padrão, prioridade fixa é igual a 0
   int dina_pri;		// prioridade dinâmica inicia igual a fixa
   unsigned int calls;
   unsigned int initialTime;
   int joinECode;
   unsigned int wakeupTime;


  //  unsigned int processTime;
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct semaphore_t
{
  task_t *queue;
  int maxSize;
  int size;

} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

typedef struct filamsg{
  struct filamsg *prev, *next;
  void *msg;
} filamsg ; 
// estrutura que define uma fila de mensagens
typedef struct
{
  int maxSize;
  int size;
  void *msg;
  semaphore_t buffer;
  semaphore_t vagas;
  semaphore_t itens;
} mqueue_t ;

#endif
