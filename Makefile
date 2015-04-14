CFLAGS  += -std=gnu11
UNAME = $(shell uname)
ifeq ($(UNAME), Linux)
LDFLAGS += -pthread
endif
all: proxyFilter decode_client_request host_connect filter
clean:
	rm -f *.o proxyFilter; rm -rf *.dSYM
proxyFilter: proxyFilter.o decode_client_request.o host_connect.o filter.o
