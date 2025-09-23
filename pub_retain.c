#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MQTT_TOPIC "upv/sci/practica2/test"

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (mosq == NULL) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    // El loop es necesario para procesar los mensajes de conexión.
    mosquitto_loop_start(mosq);
    
    printf("Esperando para conectar...\n");
    sleep(2); // Dar tiempo a que se establezca la conexión

    char line[] = "Este es un mensaje retenido.";
    printf("Publicando mensaje retenido: '%s'\n", line);

    // El último parámetro 'true' activa la retención.
    rc = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(line), line, 1, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    } else {
        printf("Mensaje publicado. El broker lo guardará.\n");
    }
    
    sleep(2); // Dar tiempo a que el mensaje se envíe

    mosquitto_loop_stop(mosq, true);
    mosquitto_disconnect(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
