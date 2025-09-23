#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define QOS_LEVEL 1
#define PING_TOPIC "pingpong/ping"
#define PONG_TOPIC "pingpong/pong"

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    if (reason_code == 0) {
        mosquitto_subscribe(mosq, NULL, PONG_TOPIC, QOS_LEVEL);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    struct timespec end_time, start_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // El payload contiene la marca de tiempo de inicio
    memcpy(&start_time, msg->payload, sizeof(struct timespec));

    double elapsed_ms = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_ms += (end_time.tv_nsec - start_time.tv_nsec) / 1000000.0;

    printf("Pong recibido. RTT: %.3f ms\n", elapsed_ms);
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;

    mosquitto_lib_init();
    mosq = mosquitto_new("ping-client", true, NULL);
    if (!mosq) return 1;

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60) != MOSQ_ERR_SUCCESS) {
        return 1;
    }
    
    mosquitto_loop_start(mosq);
    printf("Ping Client iniciado. Enviando pings cada 3 segundos (QoS=%d)...\n", QOS_LEVEL);

    while (1) {
        struct timespec start_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        
        mosquitto_publish(mosq, NULL, PING_TOPIC, sizeof(struct timespec), &start_time, QOS_LEVEL, false);
        
        sleep(3);
    }

    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
