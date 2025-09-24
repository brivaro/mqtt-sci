#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *TOPIC = "UPV/SCI/CHAT";
const int QOS = 2;

#define MAX_USERS 100

// Lista simple de usuarios conectados
char *currentUser = "anonymous";
char *usuarios[MAX_USERS];
int num_usuarios = 0;

void add_user(const char *username) {
    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i], username) == 0)
            return;
    }
    if (num_usuarios < MAX_USERS) {
        usuarios[num_usuarios++] = strdup(username);
    }
}

void remove_user(const char *username) {
    for (int i = 0; i < num_usuarios; i++) {
        if (strcmp(usuarios[i], username) == 0) {
            free(usuarios[i]);
            for (int j = i; j < num_usuarios - 1; j++) {
                usuarios[j] = usuarios[j + 1];
            }
            num_usuarios--;
            return;
        }
    }
}

void show_users() {
    printf("Usuarios conectados (%d):\n", num_usuarios);
    for (int i = 0; i < num_usuarios; i++) {
        printf(" - %s\n", usuarios[i]);
    }
}

void show_help() {
    printf("\t- /help: muestra este mensaje de ayuda\n\t- /lista: muestra todos los usuarios conectados\n\t- /privado: permite enviar un mensaje a un usuario del chat de forma privada\n\t\tuso: /privado [usuario] [mensaje]\n\t- /salir: te desconectas del chat\n");
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("Conectado al broker (%s)\n", mosquitto_connack_string(reason_code));
}

void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code) {
    printf("Desconectado del servidor\n");
}

void on_message(struct mosquitto *mosq, void *obj,
                const struct mosquitto_message *msg) {
    char *payload = (char *)msg->payload;

    if (strncmp(payload, "ONLINE:", 7) == 0) {
        add_user(payload + 7);
    } else if (strncmp(payload, "OFFLINE:", 8) == 0) {
        remove_user(payload + 8);
    } else {
        printf("%s\n", payload);
    }
}

char *get_username() {
    static char username[100] = "";
    printf("Introduce un nombre de usuario: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return NULL;
    }
    username[strcspn(username, "\n")] = '\0';
    return username;
}

int main(int argc, char *argv[]) {
    currentUser = get_username();
    if (!currentUser || strlen(currentUser) == 0) {
        printf("Nombre de usuario no válido.\n");
        return 1;
    }
    printf("Tu usuario es: %s\n", currentUser);

    struct mosquitto *mosq;
    int rc;
    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: sin memoria.\n");
        return 1;
    }

    // Mensaje de "Last Will" (desconexión forzosa)
    char will[200];
    snprintf(will, sizeof(will), "OFFLINE:%s (desconexión inesperada)", currentUser);
    mosquitto_will_set(mosq, TOPIC, strlen(will), will, 2, true);


    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_message_callback_set(mosq, on_message);

    mosquitto_loop_start(mosq);

    rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        mosquitto_destroy(mosq);
        return 1;
    }

    // Mensaje visible de inicio de sesión
    char mss_with_username[200];
    snprintf(mss_with_username, sizeof(mss_with_username),
             "ONLINE: %s", currentUser);
    mosquitto_publish(mosq, NULL, TOPIC, strlen(mss_with_username),
                      mss_with_username, QOS, false);

    // Suscripción al canal general
    mosquitto_subscribe(mosq, NULL, TOPIC, QOS);

    // Suscripción al canal privado del usuario
    char private_topic[200];
    snprintf(private_topic, sizeof(private_topic), "%s/privado/%s", TOPIC,
             currentUser);
    mosquitto_subscribe(mosq, NULL, private_topic, QOS);

    // Bucle principal de lectura de comandos
    for (;;) {
        char line[200];
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0'; 

        if (strncmp(line, "/salir", 6) == 0) {
            char msg[500];
            snprintf(msg, sizeof(msg), "OFFLINE:%s (salió voluntariamente)", currentUser);

            mosquitto_publish(mosq, NULL, TOPIC, strlen(msg), msg, QOS, false);

            // mosquitto_disconnect(mosq);
            // break;
        } else if (strncmp(line, "/privado", 8) == 0) {
            char destinatario[50], mensaje[150];
            if (sscanf(line, "/privado %49s %[^\n]", destinatario, mensaje) == 2) {
                char msg[500];
                snprintf(msg, sizeof(msg), "[PRIVADO de %s a %s]: %s", currentUser,
                         destinatario, mensaje);
                char topic_priv[500];
                snprintf(topic_priv, sizeof(topic_priv), "%s/privado/%s", TOPIC,
                         destinatario);
                mosquitto_publish(mosq, NULL, topic_priv, strlen(msg), msg, QOS, false);
            } else {
                printf("Uso correcto: /privado <usuario> <mensaje>\n");
            }
        } else if (strncmp(line, "/lista", 6) == 0) {
            show_users();
        } else if (strncmp(line, "/help", 5) == 0) {
            show_help();
        }
        else {
            if(strlen(line) == 0) {
                continue;
            }
            char buffer[250];
            snprintf(buffer, sizeof(buffer), "(%s): %s", currentUser, line);
            mosquitto_publish(mosq, NULL, TOPIC, strlen(buffer), buffer, QOS, false);
        }
    }

    mosquitto_loop_stop(mosq, false);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
