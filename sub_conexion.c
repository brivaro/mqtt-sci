#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/test"

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code == 0) {
        int rc = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
        }
    }
}

void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_disconnect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
        // Si la desconexión no fue voluntaria, intentar reconectar.
        printf("Intentando reconectar en 5 segundos...\n");
        sleep(5);
        mosquitto_reconnect(mosq);
    }
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
    printf("Suscripción exitosa (mid: %d)\n", mid);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Mensaje recibido en topic '%s': %s\n", msg->topic, (char *)msg->payload);
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
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        return 1;
    }

    // Usar mosquitto_loop_forever() que gestiona la reconexión automáticamente.
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
