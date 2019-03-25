getcontext(&a) : salva o contexto atual na variável a.
setcontext(&a) : restaura um contexto salvo anteriormente na variável a.
swapcontext(&a,&b) : salva o contexto atual em a e restaura o contexto salvo anteriormente em b.
makecontext(&a, …) : ajusta alguns valores internos do contexto salvo em a.
as variáveis a e b são do tipo ucontext_t e armazenam contextos de execução.