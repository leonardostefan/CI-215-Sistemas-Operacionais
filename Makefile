FLAGS=  -Wall -O3
DEBUG= -Wall -g
CXX=gcc
RM=rm -f
teste: testafila.c queue.c queue.h Makefile
	$(CXX) -o teste testafila.c queue.c $(FLAGS)
DEBUG: testafila.c queue.c queue.h Makefile
	$(CXX) -o teste testafila.c queue.c $(DEBUG)
clean:
	$(RM) -f teste