#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HEARTBEAT_URL "https://www.classicube.net/server/heartbeat/"
#define HEARTBEAT_INTERVAL 60

char server_salt[33] = {0};
char server_play_url[512] = {0};

static int hb_port;
static int hb_max;
static const char* hb_name;
static const char* hb_software = "sn v0.1.0";

static void generate_salt(void) {
    static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    unsigned char rnd[32];
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f || fread(rnd, 1, sizeof(rnd), f) != sizeof(rnd)) {
        for (int i = 0; i < 32; i++) rnd[i] = rand();
    }
    if (f) fclose(f);
    for (int i = 0; i < 32; i++) server_salt[i] = chars[rnd[i] % (sizeof(chars) - 1)];
    server_salt[32] = '\0';
}

static int count_players(void) {
    int n = 0;
    pthread_mutex_lock(&players_lock);
    for (int i = 0; i < 128; i++) {
        if (players[i].id != -1 && players[i].username != NULL) n++;
    }
    pthread_mutex_unlock(&players_lock);
    return n;
}

static size_t hb_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    char* out = (char*)userdata;
    size_t len = strlen(out);
    size_t n = size * nmemb;
    size_t room = 511 - len;
    if (n > room) n = room;
    memcpy(out + len, ptr, n);
    out[len + n] = '\0';
    return size * nmemb;
}

static void send_heartbeat(CURL* curl) {
    char* name_esc = curl_easy_escape(curl, hb_name, 0);
    char* soft_esc = curl_easy_escape(curl, hb_software, 0);

    char url[1024];
    snprintf(url, sizeof(url),
             HEARTBEAT_URL "?name=%s&port=%d&users=%d&max=%d&public=true&salt=%s&software=%s",
             name_esc, hb_port, count_players(), hb_max, server_salt, soft_esc);

    curl_free(name_esc);
    curl_free(soft_esc);

    char response[512] = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, hb_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Heartbeat failed: %s\n", curl_easy_strerror(res));
        return;
    }


    size_t len = strlen(response);
    while (len > 0 &&
           (response[len - 1] == '\n' || response[len - 1] == '\r' || response[len - 1] == ' '))
        response[--len] = '\0';

    if (strncmp(response, "http", 4) == 0) {
        if (strcmp(server_play_url, response) != 0) {
            strncpy(server_play_url, response, sizeof(server_play_url) - 1);
            printf("Heartbeat OK. Play URL: %s\n", server_play_url);
        }
    } else {
        fprintf(stderr, "Heartbeat error: %s\n", response);
    }
}

static void* heartbeat_thread(void* arg) {
    (void)arg;
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;

    while (1) {
        send_heartbeat(curl);
        sleep(HEARTBEAT_INTERVAL);
    }

    curl_easy_cleanup(curl);
    return NULL;
}

void heartbeat_start(int port, const char* name, int max_players) {
    hb_port = port;
    hb_name = name;
    hb_max = max_players;

    generate_salt();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    pthread_t t;
    pthread_create(&t, NULL, heartbeat_thread, NULL);
    pthread_detach(t);
}

#endif
