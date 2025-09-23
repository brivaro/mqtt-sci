#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/test"
#define WILL_TOPIC "upv/sci/practica2/test"
#define WILL_MESSAGE "Me las piro vampiro"

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    // Configurar el testamento ANTES de conectar.
    // QoS=1, retain=true
    rc = mosquitto_will_set(mosq, WILL_TOPIC, strlen(WILL_MESSAGE), WILL_MESSAGE, 1, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error setting will: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60); // Keepalive de 60s
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    printf("Conectado. Si este programa se cierra de forma abrupta (Ctrl+C),\n");
    printf("el testamento se publicará en '%s' tras ~60 segundos.\n", WILL_TOPIC);
    printf("Presiona Ctrl+C para probar.\n");

    // Mantener el programa vivo
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
