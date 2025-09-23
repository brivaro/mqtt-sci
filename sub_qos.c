#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/test"
#define CLIENT_ID "subscriber-p4-persistent" // ID Fijo y único

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code == 0) {
        // Suscribirse con QoS 1
        mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 1);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Mensaje pendiente recibido: %s\n", (char *)msg->payload);
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    
    mosquitto_lib_init();

    // 1. Client ID Fijo, 2. clean_session = false
    mosq = mosquitto_new(CLIENT_ID, false, NULL);
    if (mosq == NULL) {
        return 1;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60) != MOSQ_ERR_SUCCESS) {
        return 1;
    }

    printf("Suscriptor conectado. Recibiendo mensajes...\n");
    printf("Prueba a parar este programa, ejecutar el publicador, y volver a iniciarlo.\n");
    
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
