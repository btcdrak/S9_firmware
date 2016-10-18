OBJECTS = miner-monitor

all:$(OBJECTS)

PKG_NAME=miner-monitor

$(OBJECTS): client.o read_conf.o cgminer_api.o cjson.o gen_middata.o openssl.o rev_queue.o send_pkg.o write_conf.o dataexchange.o
	$(CC) $(LDFLAGS) $^ -o $@ -lcurl -lcrypto -lpthread -lm -lssl

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	@-rm *.o $(OBJECTS)
