CC = gcc
MOSQUITTO = $(HOME)/W/SCI+/mosquitto-2.0.18
CFLAGS = -I$(MOSQUITTO)/include
# LDFLAGS = -L$(MOSQUITTO)/lib -lmosquitto -lssl -lcrypto
LDFLAGS = $(MOSQUITTO)/lib/libmosquitto.so.1

all: practica1 practica2_pub_conexion practica2_sub_conexion practica2_pub_retain practica2_pub_will practica2_pub_qos practica2_sub_qos practica2_pub_auth practica2_sub_auth practica2_pub_ping practica2_sub_ping

practica1: chat_mqtt.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_pub_conexion: pub_conexion.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_sub_conexion: sub_conexion.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_pub_retain: pub_retain.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_pub_will: pub_will.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_pub_qos: pub_qos.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_sub_qos: sub_qos.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

practica2_pub_auth: pub_auth.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_sub_auth: sub_auth.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

practica2_pub_ping: pub_ping.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
practica2_sub_ping: sub_ping.c 
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@
	
clean: rm -f practica1 practica2_pub_conexion practica2_sub_conexion practica2_pub_retain practica2_pub_will practica2_sub_qos practica2_pub_qos practica2_pub_auth practica2_sub_auth practica2_pub_ping practica2_sub_ping
