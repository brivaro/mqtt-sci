#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MQTT_TOPIC "upv/sci/practica2/test"

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
}

void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_disconnect: %s\n", mosquitto_connack_string(reason_code));
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        return 1;
    }

    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        return 1;
    }

    time_t last_publish = 0;
    int msg_count = 0;
    while (1) {
        time_t t = time(NULL);
        if (t > last_publish + 5) { // Publicar cada 5 segundos
            char line[100];
            snprintf(line, sizeof(line), "message %d", msg_count);
            printf("Publicando: '%s'\n", line);
            
            // QoS 0: "Fire and forget", los mensajes se perderán si no hay conexión.
            rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(line), line, 1, false);
            if (rc != MOSQ_ERR_SUCCESS) {
                fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
            }
            last_publish = t;
            msg_count++;
        }
        sleep(1);
    }

    mosquitto_disconnect(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
