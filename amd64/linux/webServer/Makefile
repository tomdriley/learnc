server: server.o
	gcc $+ -o $@

server.o: server.c
	gcc -c $< -o $@

clean:
	rm -f *.o server
