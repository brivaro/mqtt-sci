#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/auth"

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) return 1;

    // Establecer usuario y contraseña
    rc = mosquitto_username_pw_set(mosq, "rw", "readwrite");
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error setting username/password: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    // Conectar al puerto 1884
    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1884, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    
    mosquitto_loop_start(mosq);
    printf("Conectado al puerto 1884 con autenticación.\n");

    char payload[] = "Mensaje autenticado y retenido";
    mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 1, true);
    printf("Mensaje publicado.\n");
    sleep(2);

    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, true);
    mosquitto_lib_cleanup();
    return 0;
}
