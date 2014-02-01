CCC = gcc
CCFLAGS = 
OBJS = main.o
SOURCE = types.h hashmap.h hashmap.c tokens.h symbol_table.h symbol_table.c scanner.h scanner.c scanner.h 
RUNFLAGS = 

$(OBJS): $(SOURCE)
	$(CCC) $(CCFLAGS) -c $(SOURCE)

compiler: $(OBJS)
	gcc hashmap.c symbol_table.c scanner.c main.c -o compiler
	#$(CCC) $(CCFLAGS) -o compiler main.c -L $(OBJS)

clean:
	rm -f main.o core compiler stutest1.out
	ls

stutest.out: compiler
	cat stutest1.in
	-compiler stutest1.in > stutest1.out
	cat stutest1.out

proftest.out: compiler
	cat $(PROFTEST)
	compiler $(PROFTEST) > proftest.out
	cat proftest.out

