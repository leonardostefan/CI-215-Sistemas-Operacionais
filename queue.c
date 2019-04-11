/*Projeto 0 "warm up" da disciplina de Sistemas Operacionais CI215 2019/1
*Alunos:
*GRR20160169 - Cesar Augusto Alves Camillo (caac16)
*sGRR20163052 - Leonardo Stefan (ls16)
*/
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

//Debug Collors
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

//Mensagens de erro no debug são printadas com esta função 
//Por algum motivo 'inline void' não funciona pra compilar 
void messagePrint2(char* color,char* erro, char* message){
    #ifdef DEBUG
        printf("%s%s: %s%s\n", color, erro, RESET, message );
    #endif
}
  

queue_t *queue_remove(queue_t **queue, queue_t *elem)
{
    // Remove o elemento indicado da fila, sem o destruir.
    // Condicoes a verificar, gerando msgs de   erro:
    // - a fila deve existir
    // - a fila nao deve estar vazia
    if (queue == NULL || (*queue) == NULL)
    {
        
        messagePrint2(YEL, "queue_remove erro","a fila não existe ou esta vazia");
        

        return (NULL);
    }

    // - o elemento deve existir
    if (elem == NULL)
    {
        
        messagePrint2(YEL, "queue_remove erro","elemento não existe");
        
        return (NULL);
    }
    // - o elemento deve pertencer a fila indicada
    int contem = 0;
    for (queue_t *aux = *queue; ((aux->next != *queue) && (contem == 0)); aux = aux->next)
    {
        if (aux == elem)
        {
            contem = 1;
        }
    }
    if (elem == *queue || elem == (*queue)->prev){
        contem=1;
    }
        if (contem == 0)
        {

            
            messagePrint2(YEL, "queue_remove erro","elemento não esta na fila");
        
            return (NULL);
        }

    // Retorno: apontador para o elemento removido, ou NULL se  erro
    if (elem->next != elem && elem->prev != elem)
    {
        elem->next->prev = elem->prev;
        elem->prev->next = elem->next;
        if (*queue == elem)
        {
            *queue = elem->next;
        }
    }

    else
    {
        *queue = NULL;
    }
    elem->next = NULL;
    elem->prev = NULL;
    return elem;
}
void queue_append(queue_t **queue, queue_t *elem)
{
    //Validando dados
    // - a fila deve existir
    if (queue == NULL)
    { //talvez isso bugue (fila vazia)
        
        messagePrint2(YEL, "queue_append erro","a fila não existe");
        
        return ;
    }
    // - o elemento deve existir
    if (elem == NULL)
    {
        
        messagePrint2(YEL, "queue_append erro","elemento não existe");
        
        return ;
    }
    // - o elemento nao deve estar em outra fila
    if (elem->prev != NULL || elem->next != NULL)
    {
        messagePrint2(YEL, "queue_append erro", "elemento já esta em uma fila" );

        return ;
    }
    if (*queue != NULL)
    {
        elem->next = *queue;
        elem->prev = (*queue)->prev;
        (*queue)->prev->next = elem;
        (*queue)->prev = elem;
    }
    else
    {
        elem->next = elem;
        elem->prev = elem;
        *queue = elem;
    }

    return;
}
int queue_size(queue_t *queue)
{
    int size = 0;
    queue_t *aux = queue;
    if (queue != NULL)
    {
        size++;
        if (queue->next != queue)
        {
            for (size = 1; aux->next != queue; aux = aux->next, size++)
                ;
        }
        else
        {
            return 1;
        }
    }
    return size;
}
void queue_print(char *name, queue_t *queue, void print_elem(void *))
{
    if (queue != NULL)
    {

        if (queue->next != queue)
        {
            for (queue_t *aux = queue; aux->next != queue; aux=aux->next)
            {
                print_elem(aux);
            }
        }
        else
        {
            print_elem(queue);
        }
    }
}
