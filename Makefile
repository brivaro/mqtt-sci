CC = gcc
MOSQUITTO = $(HOME)/clib/mosquitto-2.0.22
CFLAGS = -I$(MOSQUITTO)/include
#LDFLAGS = -L$(MOSQUITTO)/lib -lmosquitto -lssl -lcrypto
LDFLAGS = $(MOSQUITTO)/lib/libmosquitto.so.1

chat.out: chat.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
