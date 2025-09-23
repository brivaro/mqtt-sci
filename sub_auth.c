#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MQTT_TOPIC "upv/sci/practica2/auth"

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code == 0) {
        // Suscribirse al topic después de una conexión exitosa
        int rc = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
        }
    }
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
    printf("Suscrito al topic '%s'.\n", MQTT_TOPIC);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Mensaje autenticado recibido en '%s': %s\n", msg->topic, (char *)msg->payload);
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    // 1. Establecer usuario y contraseña
    rc = mosquitto_username_pw_set(mosq, "rw", "readwrite");
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error setting username/password: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    // Asignar los callbacks
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);

    // 2. Conectar al puerto 1884
    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1884, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    
    printf("Conectado al puerto 1884 con autenticación. Esperando mensajes...\n");

    // Mantener el programa escuchando mensajes
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
