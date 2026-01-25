// Minimaler DNS-Server für Captive Portal (alle Anfragen auf AP-IP)
// Quelle: https://github.com/espressif/esp-idf/blob/master/examples/protocols/sntp/main/dns_server.c (angepasst)
#include <arpa/inet.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#define DNS_PORT 53
#define DNS_MAX_LEN 512
static const char *TAG = "dns_hijack";

static char s_ap_ip[16] = "192.168.4.1"; // Default AP-IP, ggf. dynamisch setzen

void dns_set_ap_ip(const char *ip)
{
    strncpy(s_ap_ip, ip, sizeof(s_ap_ip) - 1);
    s_ap_ip[sizeof(s_ap_ip) - 1] = 0;
}

static void dns_server_task(void *pvParameters)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    uint8_t buf[DNS_MAX_LEN];
    while (1)
    {
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        int len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
        if (len < 0)
            continue;
        // DNS Header: 12 bytes, Antwort-Flag setzen
        buf[2] |= 0x80; // QR=1 (Antwort)
        buf[3] = 0x80;  // RA=1, RCODE=0
        // Fragen: 1, Antworten: 1
        buf[7] = 1;
        // Antwort anhängen (Name Pointer auf Frage)
        int pos = len;
        buf[pos++] = 0xC0;
        buf[pos++] = 0x0C; // Name pointer
        buf[pos++] = 0x00;
        buf[pos++] = 0x01; // Type A
        buf[pos++] = 0x00;
        buf[pos++] = 0x01; // Class IN
        buf[pos++] = 0x00;
        buf[pos++] = 0x00;
        buf[pos++] = 0x00;
        buf[pos++] = 0x3C; // TTL 60s
        buf[pos++] = 0x00;
        buf[pos++] = 0x04; // Data length
        inet_pton(AF_INET, s_ap_ip, &buf[pos]);
        pos += 4;
        sendto(sock, buf, pos, 0, (struct sockaddr *)&from, fromlen);
    }
    close(sock);
    vTaskDelete(NULL);
}

void dns_server_start(const char *ap_ip)
{
    dns_set_ap_ip(ap_ip);
    xTaskCreate(dns_server_task, "dns_server", 4096, NULL, 3, NULL);
}
