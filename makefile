CCC = gcc
CCFLAGS = 
OBJS = main.o
SOURCE = types.h hashmap.h hashmap.c tokens.h tokens.c symbol_table.h symbol_table.c scanner.h scanner.c parser.h parser.c code_gen.h code_gen.c 
RUNFLAGS = 
TESTS = test1.txt test2.txt test3.txt test4.txt test5.txt test6.txt test7.txt test8.txt test9.txt test10.txt test11.txt test12.txt test13.txt test14.txt test15.txt test16.txt test17.txt test18.txt test19.tx test20.txt test21.txt test22.txt test23.txt test24.txt test 25.txt test26.txt test27.txt test28.txt test29.txt test30.txt test31.txt test32.txt test33.txt test34.txt test35.txt test36.txt test37.txt test38.txt test39.txt test40.txt test41.txt

$(OBJS): $(SOURCE)
	$(CCC) $(CCFLAGS) -c $(SOURCE)

compiler: $(OBJS)
	gcc hashmap.c tokens.c symbol_table.c scanner.c parser.c code_gen.c main.c -o compiler
	#$(CCC) $(CCFLAGS) -o compiler main.c -L $(OBJS)

clean:
	rm -f main.o core compiler stutest1.out stutest2.out stutest3.out stutest4.out stutest5.out stutest6.out stutest7.out stutest8.out stutest9.out stutest10.out stutest11.out stutest12.out stutest13.out stutest14.out stutest15.out stutest16.out stutest17.out stutest18.out stutest19.out stutest20.out stutest21.out stutest22.out stutest23.out hashmap.o hashmap.h.gch tokens.o tokens.h.gch symbol_table.o symbol_table.h.gch scanner.o scanner.h.gch parser.o parser.h.gch types.h.gch test1.fs test2.fs test3.fs test4.fs test5.fs test6.fs test7.fs test8.fs test9.fs test10.fs test11.fs test12.fs test13.fs test14.fs test15.fs test16.fs test17.fs test18.fs test19.fs test20.fs test21.fs test22.fs test23.fs test24.fs test25.fs test26.fs test27.fs test28.fs test29.fs test30.fs test31.fs test32.fs test33.fs test34.fs test35.fs test36.fs test37.fs test38.fs test39.fs test40.fs test41.fs stutest24.out stutest25.out stutest26.out stutest27.out stutest28.out stutest29.out stutest30.out stutest31.out stutest32.out stutest33.out stutest34.out stutest34.out stutest35.out stutest36.out stutest37.out stutest38.out stutest39.out stutest40.out stutest41.out stutest42.out stutest43.out stutest44.out test42.fs test43.fs test44.fs stutest_fail1.out stutest_fail2.out stutest_fail3.out stutest_fail4.out stutest_fail5.out stutest_fail6.out stutest_fail7.out stutest_fail8.out stutest_fail9.out stutest_fail10.out stutest_fail11.out stutest_fail12.out stutest_fail13.out code_gen.h.gch code_gen.o test45.fs stutest45.out stutest46.out test46.fs stutest_fail14.out stutest_fail15.out stutest_fail15.out stutest_fail16.out stutest_fail17.out stutest_fail18.out stutest_fail19.out stutest_fail20.out stutest_fail21.out stutest_fail22.out stutest_fail23.out stutest_fail24.out stutest_fail25.out stutest_fail26.out stutest_fail27.out stutest_fail28.out stutest_fail29.out stutest_fail30.out test47.fs stutest47.out 
	ls

stutest.out: compiler
	cat test1.txt
	-compiler test1.txt > stutest1.out
	gforth test1.fs -e bye
	cat stutest1.out
	cat test1.fs
	cat test2.txt
	-compiler test2.txt > stutest2.out
	gforth test2.fs -e bye
	cat stutest2.out
	cat test3.txt
	-compiler test3.txt > stutest3.out
	gforth test3.fs -e bye
	cat stutest3.out
	cat test4.txt
	-compiler test4.txt > stutest4.out
	gforth test4.fs -e bye
	cat stutest4.out
	cat test5.txt
	-compiler test5.txt > stutest5.out
	gforth test5.fs -e bye
	cat stutest5.out
	cat test6.txt
	-compiler test6.txt > stutest6.out
	gforth test6.fs -e bye
	cat stutest6.out
	cat test7.txt
	-compiler test7.txt > stutest7.out
	gforth test7.fs -e bye
	cat stutest7.out
	cat test8.txt
	-compiler test8.txt > stutest8.out
	gforth test8.fs -e bye
	cat stutest8.out
	cat test9.txt
	-compiler test9.txt > stutest9.out
	gforth test9.fs -e bye
	cat stutest9.out
	cat test10.txt
	-compiler test10.txt > stutest10.out
	gforth test10.fs -e bye
	cat stutest10.out
	cat test11.txt
	-compiler test11.txt > stutest11.out
	gforth test11.fs -e bye
	cat stutest11.out
	cat test12.txt
	-compiler test12.txt > stutest12.out
	gforth test12.fs -e bye
	cat stutest12.out
	cat test13.txt
	-compiler test13.txt > stutest13.out
	gforth test13.fs -e bye
	cat stutest13.out
	cat test14.txt
	-compiler test14.txt > stutest14.out
	gforth test14.fs -e bye
	cat stutest14.out
	cat test15.txt
	-compiler test15.txt > stutest15.out
	gforth test15.fs -e bye
	cat stutest15.out
	cat test16.txt
	-compiler test16.txt > stutest16.out
	gforth test16.fs -e bye
	cat stutest16.out
	cat test17.txt
	-compiler test17.txt > stutest17.out
	gforth test17.fs -e bye
	cat stutest17.out
	cat test18.txt
	-compiler test18.txt > stutest18.out
	gforth test18.fs -e bye
	cat stutest18.out
	cat test19.txt
	-compiler test19.txt > stutest19.out
	gforth test19.fs -e bye
	cat stutest19.out
	cat test20.txt
	-compiler test20.txt > stutest20.out
	gforth test20.fs -e bye
	cat stutest20.out
	cat test21.txt
	-compiler test21.txt > stutest21.out
	gforth test21.fs -e bye
	cat stutest21.out
	cat test22.txt
	-compiler test22.txt > stutest22.out
	gforth test22.fs -e bye
	cat stutest22.out
	cat test23.txt
	-compiler test23.txt > stutest23.out
	gforth test23.fs -e bye
	cat stutest23.out
	cat test24.txt
	-compiler test24.txt > stutest24.out
	gforth test24.fs -e bye
	cat stutest24.out
	cat test25.txt
	-compiler test25.txt > stutest25.out
	gforth test25.fs -e bye
	cat stutest25.out
	cat test26.txt
	-compiler test26.txt > stutest26.out
	gforth test26.fs -e bye
	cat stutest26.out
	cat test27.txt
	-compiler test27.txt > stutest27.out
	gforth test27.fs -e bye
	cat stutest27.out
	cat test28.txt
	-compiler test28.txt > stutest28.out
	gforth test28.fs -e bye
	cat stutest28.out
	cat test29.txt
	-compiler test29.txt > stutest29.out
	gforth test29.fs -e bye
	cat stutest29.out
	cat test30.txt
	-compiler test30.txt > stutest30.out
	gforth test30.fs -e bye
	cat stutest30.out
	cat test31.txt
	-compiler test31.txt > stutest31.out
	gforth test31.fs -e bye
	cat stutest31.out
	cat test32.txt
	-compiler test32.txt > stutest32.out
	gforth test32.fs -e bye
	cat stutest32.out
	cat test33.txt
	-compiler test33.txt > stutest33.out
	gforth test33.fs -e bye
	cat stutest33.out
	cat test34.txt
	-compiler test34.txt > stutest34.out
	gforth test34.fs -e bye
	cat stutest34.out
	cat test35.txt
	-compiler test35.txt > stutest35.out
	gforth test35.fs -e bye
	cat stutest35.out
	cat test36.txt
	-compiler test36.txt > stutest36.out
	gforth test36.fs -e bye
	cat stutest36.out
	cat test37.txt
	-compiler test37.txt > stutest37.out
	gforth test37.fs -e bye
	cat stutest37.out
	cat test38.txt
	-compiler test38.txt > stutest38.out
	gforth test38.fs -e bye
	cat stutest38.out
	cat test39.txt
	-compiler test39.txt > stutest39.out
	gforth test39.fs -e bye
	cat stutest39.out
	cat test40.txt
	-compiler test40.txt > stutest40.out
	gforth test40.fs -e bye
	cat stutest40.out
	cat test41.txt
	-compiler test41.txt > stutest41.out
	gforth test41.fs -e bye
	cat stutest41.out
	cat test42.txt
	-compiler test42.txt > stutest42.out
	gforth test42.fs -e bye
	cat stutest42.out
	cat test43.txt
	-compiler test43.txt > stutest43.out
	gforth test43.fs -e bye
	cat stutest43.out
	cat test44.txt
	-compiler test44.txt > stutest44.out
	gforth test44.fs -e bye
	cat stutest44.out
	cat test45.txt
	-compiler test45.txt > stutest45.out
	gforth test45.fs -e bye
	cat stutest45.out
	cat test46.txt
	-compiler test46.txt > stutest46.out
	gforth test46.fs -e bye
	cat stutest46.out
	cat test47.txt
	-compiler test47.txt > stutest47.out
	gforth test47.fs -e bye
	cat stutest47.out
	cat test_fail1.txt
	-compiler test_fail1.txt > stutest_fail1.out
	cat stutest_fail1.out
	cat test_fail2.txt
	-compiler test_fail2.txt > stutest_fail2.out
	cat stutest_fail2.out
	cat test_fail3.txt
	-compiler test_fail3.txt > stutest_fail3.out
	cat stutest_fail3.out
	cat test_fail4.txt
	-compiler test_fail4.txt > stutest_fail4.out
	cat stutest_fail4.out
	cat test_fail5.txt
	-compiler test_fail5.txt > stutest_fail5.out
	cat stutest_fail5.out
	cat test_fail6.txt
	-compiler test_fail6.txt > stutest_fail6.out
	cat stutest_fail6.out
	cat test_fail7.txt
	-compiler test_fail7.txt > stutest_fail7.out
	cat stutest_fail7.out
	cat test_fail8.txt
	-compiler test_fail8.txt > stutest_fail8.out
	cat stutest_fail8.out
	cat test_fail9.txt
	-compiler test_fail9.txt > stutest_fail9.out
	cat stutest_fail9.out
	cat test_fail10.txt
	-compiler test_fail10.txt > stutest_fail10.out
	cat stutest_fail10.out
	cat test_fail11.txt
	-compiler test_fail11.txt > stutest_fail11.out
	cat stutest_fail11.out
	cat test_fail12.txt
	-compiler test_fail12.txt > stutest_fail12.out
	cat stutest_fail12.out
	cat test_fail13.txt
	-compiler test_fail13.txt > stutest_fail13.out
	cat stutest_fail13.out
	cat test_fail14.txt
	-compiler test_fail14.txt > stutest_fail14.out
	cat stutest_fail14.out
	cat test_fail15.txt
	-compiler test_fail15.txt > stutest_fail15.out
	cat stutest_fail15.out
	cat test_fail16.txt
	-compiler test_fail16.txt > stutest_fail16.out
	cat stutest_fail16.out
	cat test_fail17.txt
	-compiler test_fail17.txt > stutest_fail17.out
	cat stutest_fail17.out
	cat test_fail18.txt
	-compiler test_fail18.txt > stutest_fail18.out
	cat stutest_fail18.out
	cat test_fail19.txt
	-compiler test_fail19.txt > stutest_fail19.out
	cat stutest_fail19.out
	cat test_fail20.txt
	-compiler test_fail20.txt > stutest_fail20.out
	cat stutest_fail20.out
	cat test_fail21.txt
	-compiler test_fail21.txt > stutest_fail21.out
	cat stutest_fail21.out
	cat test_fail22.txt
	-compiler test_fail22.txt > stutest_fail22.out
	cat stutest_fail22.out
	cat test_fail23.txt
	-compiler test_fail23.txt > stutest_fail23.out
	cat stutest_fail23.out
	cat test_fail24.txt
	-compiler test_fail24.txt > stutest_fail24.out
	cat stutest_fail24.out
	cat test_fail25.txt
	-compiler test_fail25.txt > stutest_fail25.out
	cat stutest_fail25.out
	cat test_fail26.txt
	-compiler test_fail26.txt > stutest_fail26.out
	cat stutest_fail26.out
	cat test_fail27.txt
	-compiler test_fail27.txt > stutest_fail27.out
	cat stutest_fail27.out
	cat test_fail28.txt
	-compiler test_fail28.txt > stutest_fail28.out
	cat stutest_fail28.out
	cat test_fail29.txt
	-compiler test_fail29.txt > stutest_fail29.out
	cat stutest_fail29.out
	cat test_fail30.txt
	-compiler test_fail30.txt > stutest_fail30.out
	cat stutest_fail30.out

proftest.out: compiler
	cat $(PROFTEST)
	compiler $(PROFTEST) > proftest.out
	cat proftest.out

