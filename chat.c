#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const char *TOPIC = "UPV/SCI";
const int QOS = 2;

void on_connect(struct mosquitto *mosq, void *obj, int reason_code) {
  printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
}
void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code) {
  printf("on_disconnect: %s\n", mosquitto_connack_string(reason_code));
}


char *get_username() {
  static char username[100] = "";

  printf("Enter a username: ");
  if (fgets(username, sizeof(username), stdin) == NULL) {
    return NULL;
  }
  username[strcspn(username, "\n")] = '\0';
  return username;
}

int main(int argc, char *argv[]) {
  char *username = get_username();
  printf("tu usuario %s\n", username);
  struct mosquitto *mosq;
  int rc;
  mosquitto_lib_init();
  mosq = mosquitto_new(NULL, true, NULL);
  if (mosq == NULL) {
    fprintf(stderr, "Error: Out of memory.\n");
    return 1;
  }

  char will[200];
  sprintf(will, "El usuario %s ha cerrado sesión de forma abrupta", username);

  mosquitto_will_set(mosq, TOPIC, strlen(will), will, QOS, false);
  mosquitto_connect_callback_set(mosq, on_connect);
  mosquitto_disconnect_callback_set(mosq, on_disconnect);
  rc = mosquitto_loop_start(mosq);
  if (rc != MOSQ_ERR_SUCCESS) {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return 1;
  }
  rc = mosquitto_connect(mosq, "test.mosquitto.org", 1883, 60);
  if (rc != MOSQ_ERR_SUCCESS) {
    mosquitto_destroy(mosq);
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    return 1;
  }
  for (;;) {
    char line[100];
    if (fgets(line, 100, stdin) == NULL)
      break;
    char buffer[200];
    sprintf(buffer, "(%s): %s", username, line);
    rc = mosquitto_publish(mosq, NULL, TOPIC, strlen(buffer), buffer, QOS,
                           false);
    if (rc != MOSQ_ERR_SUCCESS) {
      fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    }
  }
  //  /* main loop */
  //  time_t last_publish = 0;
  //  int msg_count = 0;
  //
  //  while (1) {
  //      time_t t = time(NULL);
  //      if (t > last_publish + 10) {
  //          char line[100];
  //          snprintf(line, sizeof(line), "message %d", msg_count);
  //          printf("Publicando: ’%s’\n", line);
  //          int rc = mosquitto_publish(mosq, NULL, TOPIC, strlen(line), line,
  //                                     0, true);
  //          if (rc != MOSQ_ERR_SUCCESS) {
  //              fprintf(stderr, "Error publishing: %s\n",
  //              mosquitto_strerror(rc));
  //          }
  //          last_publish = t;
  //          msg_count++;
  //      }
  //  }
  //
  //  mosquitto_disconnect(mosq);
  //  mosquitto_lib_cleanup();
  //  return 0;
}
