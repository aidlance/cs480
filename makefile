CCC = gcc
CCFLAGS = 
OBJS = main.o
SOURCE = types.h hashmap.h hashmap.c tokens.h tokens.c symbol_table.h symbol_table.c scanner.h scanner.c parser.h parser.c 
RUNFLAGS = 

$(OBJS): $(SOURCE)
	$(CCC) $(CCFLAGS) -c $(SOURCE)

compiler: $(OBJS)
	gcc hashmap.c tokens.c symbol_table.c scanner.c parser.c main.c -o compiler
	#$(CCC) $(CCFLAGS) -o compiler main.c -L $(OBJS)

clean:
	rm -f main.o core compiler stutest1.out stutest2.out stutest3.out stutest4.out stutest5.out stutest6.out stutest7.out stutest8.out stutest9.out stutest10.out stutest11.out stutest12.out stutest13.out stutest14.out stutest15.out stutest16.out stutest17.out stutest18.out stutest19.out stutest20.out stutest21.out stutest22.out stutest23.out hashmap.o hashmap.h.gch tokens.o tokens.h.gch symbol_table.o symbol_table.h.gch scanner.o scanner.h.gch parser.o parser.h.gch types.h.gch
	ls

stutest.out: compiler
	cat test1.txt
	-compiler test1.txt > stutest1.out
	cat stutest1.out
	cat test2.txt
	-compiler test2.txt > stutest2.out
	cat stutest2.out
	cat test3.txt
	-compiler test3.txt > stutest3.out
	cat stutest3.out
	cat test4.txt
	-compiler test4.txt > stutest4.out
	cat stutest4.out
	cat test5.txt
	-compiler test5.txt > stutest5.out
	cat stutest5.out
	cat test6.txt
	-compiler test6.txt > stutest6.out
	cat stutest6.out
	cat test7.txt
	-compiler test7.txt > stutest7.out
	cat stutest7.out
	cat test8.txt
	-compiler test8.txt > stutest8.out
	cat stutest8.out
	cat test9.txt
	-compiler test9.txt > stutest9.out
	cat stutest9.out
	cat test10.txt
	-compiler test10.txt > stutest10.out
	cat stutest10.out
	cat test11.txt
	-compiler test11.txt > stutest11.out
	cat stutest11.out
	cat test12.txt
	-compiler test12.txt > stutest12.out
	cat stutest12.out
	cat test13.txt
	-compiler test13.txt > stutest13.out
	cat stutest13.out
	cat test14.txt
	-compiler test14.txt > stutest14.out
	cat stutest14.out
	cat test15.txt
	-compiler test15.txt > stutest15.out
	cat stutest15.out
	cat test16.txt
	-compiler test16.txt > stutest16.out
	cat stutest16.out
	cat test17.txt
	-compiler test17.txt > stutest17.out
	cat stutest17.out
	cat test18.txt
	-compiler test18.txt > stutest18.out
	cat stutest18.out
	cat test19.txt
	-compiler test19.txt > stutest19.out
	cat stutest19.out
	cat test20.txt
	-compiler test20.txt > stutest20.out
	cat stutest20.out
	cat test21.txt
	-compiler test21.txt > stutest21.out
	cat stutest21.out
	cat test22.txt
	-compiler test22.txt > stutest22.out
	cat stutest22.out
	cat test23.txt
	-compiler test23.txt > stutest23.out
	cat stutest23.out

proftest.out: compiler
	cat $(PROFTEST)
	compiler $(PROFTEST) > proftest.out
	cat proftest.out

