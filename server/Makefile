obj = ../base_struct/hdict.o \
	../base_struct/Hheap.o \
	../base_struct/hskiplist.o \
	../base_struct/hlist.o \
	../base_struct/sds.o \
	../base_net/hnet.o \
	../eventloop/ae.o 

all:db.c networking.c Server.c redisbase.a
	gcc -g db.c networking.c Server.c redisbase.a -o h_redis
redisbase.a:$(obj)
	ar crv redisbase.a ../base_struct/*.o ../base_net/*.o ../eventloop/*.o ../redis_src/*.o 

%.o: %.c
	gcc -g  -c $^ -o $@

clean:
	rm -rf h_redis redisbase.a $(obj)
