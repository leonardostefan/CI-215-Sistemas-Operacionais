FLAGS=  -Wall -O3
DEBUG= -Wall -g -DDEBUG
CXX=gcc
RM=rm -f
ppos_core: 
	$(CXX) -o task1_exe ppos_core.c pingpong-task1.c queue.c $(DEBUG)
	$(CXX) -o task2_exe ppos_core.c pingpong-task2.c queue.c $(DEBUG)
	$(CXX) -o task3_exe ppos_core.c pingpong-task3.c queue.c $(DEBUG)
	$(CXX) -o taskdispatcher_exe ppos_core.c pingpong-dispatcher.c queue.c $(DEBUG)
	$(CXX) -o taskscheduler_exe ppos_core.c pingpong-scheduler.c queue.c $(DEBUG)
	$(CXX) -o taskpreempcao_exe ppos_core.c pingpong-preempcao.c queue.c $(DEBUG)
	$(CXX) -o P6_1_exe ppos_core.c P6_1.c queue.c $(DEBUG)
	$(CXX) -o P6_2_exe ppos_core.c P6_2.c queue.c $(DEBUG)
	$(CXX) -o P7_exe ppos_core.c P7.c queue.c $(DEBUG)

queue: testafila.c queue.c queue.h Makefile
	$(CXX) -o teste testafila.c queue.c $(FLAGS)

exe_ppos_core: 
	$(CXX) -o task1_exe ppos_core.c pingpong-task1.c queue.c $(FLAGS)
	$(CXX) -o task2_exe ppos_core.c pingpong-task2.c queue.c $(FLAGS)
	$(CXX) -o task3_exe ppos_core.c pingpong-task3.c queue.c $(FLAGS)
	$(CXX) -o taskdispatcher_exe ppos_core.c pingpong-dispatcher.c queue.c $(FLAGS)
	$(CXX) -o taskscheduler_exe ppos_core.c pingpong-scheduler.c queue.c $(FLAGS)
	$(CXX) -o taskpreempcao_exe ppos_core.c pingpong-preempcao.c queue.c $(FLAGS)	
	$(CXX) -o P6_1_exe ppos_core.c P6_1.c queue.c $(FLAGS)
	$(CXX) -o P6_2_exe ppos_core.c P6_2.c queue.c $(FLAGS)
	$(CXX) -o P7_exe ppos_core.c P7.c queue.c $(FLAGS)



clean:
	$(RM) -f *_exe 