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
            // 10초마다 이미지 자동 새로고침
            setInterval(refreshImage, 10000);
        }
    </script>
</body>
</html>
)HTML";
    
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

// SD카드에서 이미지 읽어서 전송
static esp_err_t image_handler(httpd_req_t *req)
{
    // SD 카드 마운트
    esp_err_t ret = bsp_sdcard_mount();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // 이미지 파일 열기
    FILE *f = fopen("/sdcard/cat_detection_result.jpg", "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open image file");
        bsp_sdcard_unmount();
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // 파일 크기 확인
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // 버퍼 할당
    uint8_t *buf = (uint8_t*)malloc(file_size);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        fclose(f);
        bsp_sdcard_unmount();
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // 파일 읽기
    size_t read_size = fread(buf, 1, file_size, f);
    fclose(f);
    bsp_sdcard_unmount();
    
    if (read_size != file_size) {
        ESP_LOGE(TAG, "Failed to read complete file");
        free(buf);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // HTTP 헤더 설정
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=detection_result.jpg");
    
    // 이미지 전송
    esp_err_t res = httpd_resp_send(req, (const char *)buf, file_size);
    free(buf);
    
    return res;
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