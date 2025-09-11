#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "bsp/esp-bsp.h"
#include "wifi_server.hpp"

static const char *TAG = "wifi_server";

// WiFi 설정
#define WIFI_SSID     "TP-Link_54BE"
#define WIFI_PASSWORD "goldkwang1!"

// Static IP 설정
#define STATIC_IP_ADDR        "192.168.0.11"
#define STATIC_GATEWAY_ADDR   "192.168.0.1"
#define STATIC_NETMASK_ADDR   "255.255.255.0"

// 이벤트 그룹
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// HTTP 서버 핸들
static httpd_handle_t server = NULL;

// WiFi 이벤트 핸들러
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Reconnecting...");
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// 메인 페이지 핸들러
static esp_err_t index_handler(httpd_req_t *req)
{
    const char* html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Golf Ball Detection</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        h1 {
            color: #333;
            text-align: center;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        #image {
            width: 100%;
            max-width: 640px;
            height: auto;
            display: block;
            margin: 20px auto;
            border: 2px solid #ddd;
        }
        .info {
            text-align: center;
            margin: 10px 0;
            font-size: 18px;
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin: 5px;
            background-color: #4CAF50;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            cursor: pointer;
        }
        .button:hover {
            background-color: #45a049;
        }
        .center {
            text-align: center;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Golf Ball Detection System</h1>
        <div class="info" id="version">Version: Loading...</div>
        <div class="center">
            <img id="image" src="/image.jpg" alt="Detection Result">
        </div>
        <div class="info" id="status">Status: Ready</div>
        <div class="center">
            <button class="button" onclick="refreshImage()">Refresh Image</button>
        </div>
    </div>
    <script>
        function refreshImage() {
            var img = document.getElementById('image');
            img.src = '/image.jpg?' + new Date().getTime();
        }
        
        function updateVersion() {
            fetch('/version')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('version').innerHTML = 'Version: ' + data;
                });
        }
        
        // 페이지 로드시 버전 정보 가져오기
        window.onload = function() {
            updateVersion();
        }
    </script>
</body>
</html>
)HTML";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

// 전역 변수로 처리된 이미지 저장
static uint8_t *g_image_buffer = NULL;
static size_t g_image_size = 0;

// 이미지 버퍼 설정 함수
void set_image_buffer(uint8_t *buffer, size_t size) {
    if (g_image_buffer != NULL) {
        free(g_image_buffer);
    }
    g_image_buffer = buffer;
    g_image_size = size;
}

// 이미지 전송 핸들러
static esp_err_t image_handler(httpd_req_t *req)
{
    if (g_image_buffer == NULL || g_image_size == 0) {
        ESP_LOGE(TAG, "No image available");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // HTTP 헤더 설정
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    char content_length[16];
    snprintf(content_length, sizeof(content_length), "%d", g_image_size);
    httpd_resp_set_hdr(req, "Content-Length", content_length);
    
    // 이미지 전송 - 청크 단위로
    size_t sent = 0;
    size_t chunk_size = 4096;
    while (sent < g_image_size) {
        size_t to_send = ((g_image_size - sent) > chunk_size) ? chunk_size : (g_image_size - sent);
        if (httpd_resp_send_chunk(req, (const char *)(g_image_buffer + sent), to_send) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send image chunk");
            return ESP_FAIL;
        }
        sent += to_send;
    }
    
    // 청크 전송 종료
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

// 버전 정보 핸들러
static esp_err_t version_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_send(req, VERSION_STRING, strlen(VERSION_STRING));
}

// HTTP 서버 시작
static esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_open_sockets = 1;  // 단일 연결만 허용
    config.recv_wait_timeout = 10;  // 수신 대기 시간 감소
    config.send_wait_timeout = 10;  // 전송 대기 시간 감소
    
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // URI 핸들러 등록
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index_uri);
        
        httpd_uri_t image_uri = {
            .uri       = "/image.jpg",
            .method    = HTTP_GET,
            .handler   = image_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &image_uri);
        
        httpd_uri_t version_uri = {
            .uri       = "/version",
            .method    = HTTP_GET,
            .handler   = version_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &version_uri);
        
        ESP_LOGI(TAG, "Server Started");
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Error starting server!");
    return ESP_FAIL;
}

// WiFi 초기화
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    
    // 고정 IP 설정
    esp_netif_dhcpc_stop(netif);
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 0, 11);
    IP4_ADDR(&ip_info.gw, 192, 168, 0, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    esp_netif_set_ip_info(netif, &ip_info);
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASSWORD);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    
    // WiFi 연결 대기
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                          WIFI_CONNECTED_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", WIFI_SSID);
        ESP_LOGI(TAG, "IP Address: %s", STATIC_IP_ADDR);
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi");
    }
}

// WiFi 서버 시작
void start_wifi_server(void)
{
    // NVS 초기화
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // WiFi 초기화
    wifi_init_sta();
    
    // 웹 서버 시작
    start_webserver();
    
    ESP_LOGI(TAG, "WiFi server started. Access at http://%s", STATIC_IP_ADDR);
}