OBJS = pinginfoview.o
CFLAG = -lncurses

pinginfoview: pinginfoview.o
	gcc ${OBJS} -o $@ ${CFLAG}

pinginfoview.o: pinginfoview.c
	gcc -c $^

clean:
	rm -f pinginfoview pinginfoview.o
