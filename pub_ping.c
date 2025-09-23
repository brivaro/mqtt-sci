#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>

#define QOS_LEVEL 1
#define PING_TOPIC "pingpong/ping"
#define PONG_TOPIC "pingpong/pong"

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    if (reason_code == 0) {
        mosquitto_subscribe(mosq, NULL, PING_TOPIC, QOS_LEVEL);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    // Reenviar el mensaje recibido al topic de pong
    mosquitto_publish(mosq, NULL, PONG_TOPIC, msg->payloadlen, msg->payload, QOS_LEVEL, false);
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;

    mosquitto_lib_init();
    mosq = mosquitto_new("pong-responder", true, NULL);
    if (!mosq) return 1;

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60) != MOSQ_ERR_SUCCESS) {
        return 1;
    }

    printf("Pong Responder iniciado. Escuchando pings...\n");
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
