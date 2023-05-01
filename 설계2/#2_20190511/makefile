main : blank.o ssu_score.o
	gcc main.c blank.o ssu_score.o -o ssu_score
blank.o : blank.c blank.h
	gcc -c blank.c
ssu_score.o : ssu_score.c ssu_score.h blank.h
	gcc -c ssu_score.c

clean :
	rm *.o
	rm ssu_score