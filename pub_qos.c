#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/test"
#define CLIENT_ID "publisher-p4"

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(CLIENT_ID, true, NULL);
    if (mosq == NULL) {
        return 1;
    }

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        return 1;
    }
    mosquitto_loop_start(mosq);
    sleep(1);

    int i;
    for (i = 0; i < 10; ++i) {
        char payload[50];
        snprintf(payload, sizeof(payload), "Mensaje persistente #%d", i);
        printf("Publicando: %s (QoS 1)\n", payload);
        // Usamos QoS 1 para asegurar la entrega
        mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 1, true);
        sleep(2);
    }

    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, true);
    mosquitto_lib_cleanup();
    return 0;
}
