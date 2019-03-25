FLAGS=  -Wall -O3 -Wextras
DEBUG= -Wall -g -DDEBUG
CXX=gcc
RM=rm -f
ppos_core: 
	$(CXX) -o task1 ppos_core.c pingpong-task1.c  $(DEBUG)
	$(CXX) -o task2 ppos_core.c pingpong-task2.c $(DEBUG)
	$(CXX) -o task3 ppos_core.c pingpong-task3.c $(DEBUG)


queue: testafila.c queue.c queue.h Makefile
	$(CXX) -o teste testafila.c queue.c $(FLAGS)
DEBUG: 
	$(CXX) -o teste testafila.c queue.c $(DEBUG)
clean:
	$(RM) -f teste