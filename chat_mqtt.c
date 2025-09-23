#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <unistd.h>
#include <stdbool.h>

char g_nickname[50];

void reconnect(struct mosquitto *mosq);

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
    int rc;
    printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
        return;
    }
    rc = mosquitto_subscribe(mosq, NULL, "UPV/SCI/#", 1);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
        mosquitto_disconnect(mosq);
    }
}

void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code)
{
    printf("on_disconnect: %s\n", mosquitto_connack_string(reason_code));
    if (reason_code != 0) {
        printf("Intentando reconectar en 5 segundos...\n");
        sleep(5);
        reconnect(mosq);
    }
}

void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
    printf("Mensaje con mid %d ha sido publicado.\n", mid);
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    int i;
    bool has_subscription = false;
    for (i = 0; i < qos_count; i++) {
        printf("on_subscribe: %d: granted qos = %d\n", i, granted_qos[i]);
        if (granted_qos[i] <= 2) {
            has_subscription = true;
        }
    }
    if (!has_subscription) {
        fprintf(stderr, "Error: All subscriptions rejected.\n");
        mosquitto_disconnect(mosq);
    }
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    printf("\r[%s]: %s\n", msg->topic, (char *)msg->payload);
    fflush(stdout);
}

void connect_client(struct mosquitto *mosq) {
    int rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al conectar: %s\n", mosquitto_strerror(rc));
    }
}

void reconnect(struct mosquitto *mosq) {
    int rc = mosquitto_reconnect(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al reconectar: %s\n", mosquitto_strerror(rc));
    }
}

int main(int argc, char *argv[])
{
    struct mosquitto *mosq;
    int rc;
    char publish_topic[100];
    char temp_input[50];

    printf("Introduce tu nickname (ej: bvalrod): ");
    if (fgets(temp_input, sizeof(temp_input), stdin) != NULL) {
        temp_input[strcspn(temp_input, "\n")] = 0;
        if (temp_input[0] == '\0') {
            strcpy(g_nickname, "Anónimo");
        } else {
            strncpy(g_nickname, temp_input, sizeof(g_nickname) - 1);
            g_nickname[sizeof(g_nickname) - 1] = '\0';
        }
    } else {
        return 1;
    }

    printf("Introduce el sub-topic (ej: general) o presiona Enter para usar 'UPV/SCI' por defecto: ");
    if (fgets(temp_input, sizeof(temp_input), stdin) != NULL) {
        temp_input[strcspn(temp_input, "\n")] = 0;
        if (temp_input[0] == '\0') {
            strcpy(publish_topic, "UPV/SCI");
        } else {
            snprintf(publish_topic, sizeof(publish_topic), "UPV/SCI/%s", temp_input);
        }
    } else {
        return 1;
    }
    
    printf("Te conectarás como '%s'. Publicarás en el topic '%s'.\n", g_nickname, publish_topic);

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    char will_message[200];
    snprintf(will_message, sizeof(will_message), "(SISTEMA) ¡Hasta la vista, baby! %s", g_nickname);

    rc = mosquitto_will_set(mosq, "UPV/SCI/status", strlen(will_message), will_message, 1, true);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al configurar el will: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_publish_callback_set(mosq, on_publish);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect_async(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error connecting: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    rc = mosquitto_loop_start(mosq);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error starting loop: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("Conectado. Escribe un mensaje y presiona Enter para enviar. Presiona Ctrl+D para salir.\n");

    char line[100];
    char message_with_nick[200];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0) continue;

        snprintf(message_with_nick, sizeof(message_with_nick), "(%s) %s", g_nickname, line);
        rc = mosquitto_publish(mosq, NULL, publish_topic, strlen(message_with_nick), message_with_nick, 1, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
        }
    }

    printf("Cerrando...\n");
    mosquitto_disconnect(mosq);
    mosquitto_loop_stop(mosq, true);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
