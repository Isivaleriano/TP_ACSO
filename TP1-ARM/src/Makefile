OBJS = shell.c sim.c decode.c execute.c
sim: $(OBJS)
	gcc -g -O0 $(OBJS) -o sim

.PHONY: clean
clean:
	rm -rf *.o *~ sim
