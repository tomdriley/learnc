LDFLAGS = -z noexecstack

a.out: ext.o main.o
	gcc $+ $(LDFLAGS)

main.o: main.c
	gcc -c $< -o $@

ext.o: ext.s
	as $< -o $@

clean:
	rm -f *.o a.out
