all: hdict.o Hheap.o hlist.o hskiplist.o sds.o

%.o: %.c
	gcc -g  -c $^ -o $@

clean:
	rm -rf *.o

