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
#include "camera_init.hpp"
#include "esp_camera.h"

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
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "Disconnected. Reason: %d. Reconnecting...", disconnected->reason);
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Wait before reconnect
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// 메인 페이지 핸들러 - camera_web_server.cpp와 동일한 형식
static esp_err_t index_handler(httpd_req_t *req)
{
    const char* html = R"HTML(
<!DOCTYPE HTML>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32-CAM Golf Ball Detection</title>
    <style>
        /* Global styles */
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            padding: 20px;
            margin: 0;
        }
        
        /* Typography */
        h1 {
            color: #333;
            margin: 10px 0;
            text-align: center;
        }
        h2, h3 {
            margin-top: 0;
        }
        
        /* Layout containers */
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .main-content, .bottom-content {
            display: flex;
            gap: 20px;
        }
        .main-content {
            margin-bottom: 20px;
        }
        
        /* Panel styles */
        .left-panel {
            flex: 1;
            text-align: center;
        }
        .right-panel {
            flex: 1;
            text-align: center;
        }
        .bottom-left, .bottom-right {
            flex: 1;
        }
        
        /* Image styles */
        img {
            width: 100%;
            height: auto;
            border: 2px solid #ddd;
            border-radius: 5px;
            display: block;
        }
        
        /* Controls and forms */
        .controls {
            margin: 20px 0;
            padding: 15px;
            background-color: #f8f8f8;
            border-radius: 5px;
        }
        #roi-controls {
            height: auto;
            display: flex;
            flex-direction: column;
            justify-content: center;
        }
        .control-group {
            margin: 10px 0;
        }
        .control-group label {
            display: inline-block;
            width: 180px;
            text-align: right;
            margin-right: 10px;
            font-weight: bold;
        }
        .control-group label[style*="margin-left"] {
            width: 80px;
            margin-left: 20px;
        }
        input[type="number"] {
            width: 80px;
            padding: 5px;
            margin: 5px;
        }
        
        /* Info panel */
        .info {
            margin-top: 20px;
            padding: 15px;
            background-color: #e8f4f8;
            border-radius: 5px;
            height: 270px;
            overflow-y: auto;
        }
        .info p {
            margin: 5px 0;
            color: #555;
        }
        
        /* Log area */
        .log-area {
            margin-top: 20px;
            padding: 10px;
            background-color: #1e1e1e;
            border: 1px solid #444;
            border-radius: 5px;
            height: 270px;
            overflow-y: auto;
            font-family: monospace;
            font-size: 14px;
            color: #0f0;
            text-align: left;
        }
        .log-entry {
            margin: 2px 0;
        }
        .log-error {
            color: #f00;
        }
        .log-success {
            color: #0f0;
        }
        .log-info {
            color: #ff0;
        }
        .log-group-a {
            color: #4CAF50;
            background-color: rgba(76, 175, 80, 0.1);
            border-left: 3px solid #4CAF50;
            padding-left: 5px;
        }
        .log-group-b {
            color: #2196F3;
            background-color: rgba(33, 150, 243, 0.1);
            border-left: 3px solid #2196F3;
            padding-left: 5px;
        }
        
        /* Buttons */
        button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin: 5px;
        }
        button:hover {
            background-color: #45a049;
        }
        #applyButton {
            background-color: #2196F3;
        }
        #applyButton:hover {
            background-color: #0b7dda;
        }
        button[style*="#f44336"] {
            background-color: #f44336;
            padding: 5px 10px;
            font-size: 12px;
        }
        button[style*="#f44336"]:hover {
            background-color: #d32f2f;
        }
        
        /* Button row */
        .button-row {
            margin-top: 10px;
            white-space: nowrap;
        }
        .button-row button {
            display: inline-block;
        }
        
        /* Responsive design */
        @media (max-width: 1024px) {
            .main-content, .bottom-content {
                flex-direction: column;
            }
            .container {
                max-width: 100%;
                padding: 10px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32-CAM Golf Ball Detection</h1>
        
        <div class="main-content">
            <div class="left-panel">
                <h2>Live Stream <span id="frameCounter" style="color: #2196F3;">(0)</span></h2>
                <img src="/image.jpg" id="stream" style="width: 100%; height: auto;">
            </div>
            
            <div class="right-panel">
                <h2>ROI Settings</h2>
                <div class="controls" id="roi-controls">
                    <div class="control-group">
                        <label>X Offset (↑좌,↓우)</label>
                        <input type="number" id="xOffset" value="368" min="0" max="800" step="8">
                        <label style="width: 80px; margin-left: 20px;">Width</label>
                        <input type="number" id="width" value="544" min="64" max="544" step="8">
                    </div>
                    <div class="control-group">
                        <label>Y Offset (↑상,↓하)</label>
                        <input type="number" id="yOffset" value="380" min="0" max="864" step="8">
                        <label style="width: 80px; margin-left: 20px;">Height</label>
                        <input type="number" id="height" value="264" min="64" max="400" step="8">
                    </div>
                </div>
                <div class="button-row">
                    <button onclick="saveImage()">Save Image</button>
                    <button id="applyButton" onclick="applySettings()">Apply</button>
                    <button onclick="setDefault()">Default</button>
                </div>
            </div>
        </div>
        
        <div class="bottom-content">
            <div class="bottom-left">
                <div class="info">
                    <p><strong>ESP32-S3 Camera with OV2640 Sensor</strong></p>
                    <p>Resolution: SXGA (1280x1024)</p>
                    <p id="roiInfo">ROI: 544x264 @ (368, 380)</p>
                    <p>Quality: High (JPEG=10)</p>
                    <p>Stream: MJPEG</p>
                    <p id="currentSettings">Current: X=352, Y=520, Width=568, Height=264</p>
                    <p id="defaultSettings">Default: X=352, Y=520, Width=568, Height=264</p>
                    <p id="detectionStatus" style="font-weight: bold; color: #4CAF50;">Detection: Enabled</p>
                    <p id="detectionResult" style="font-weight: bold; color: #4CAF50;"></p>
                </div>
            </div>
            
            <div class="bottom-right">
                <div style="position: relative;">
                    <button onclick="clearLog()" style="position: absolute; right: 10px; top: 10px; background-color: #f44336; padding: 5px 10px; font-size: 12px;">Clear</button>
                    <div class="log-area" id="logArea">
                        <div class="log-entry log-info">System ready. Waiting for commands...</div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        // 현재 설정값을 추적하기 위한 변수
        var roi_offset_x = 352;
        var roi_offset_y = 520;
        var roi_width = 568;
        var roi_height = 264;
        var detection_interval;
        var detection_group_color = false;  // 감지 그룹 색상 토글
        
        function addLog(message, type) {
            var logArea = document.getElementById('logArea');
            var timestamp = new Date().toLocaleTimeString();
            var entry = document.createElement('div');
            entry.className = 'log-entry log-' + (type || 'info');
            entry.textContent = '[' + timestamp + '] ' + message;
            logArea.appendChild(entry);
            logArea.scrollTop = logArea.scrollHeight;
            
            // 로그가 50개를 초과하면 오래된 것부터 제거
            while (logArea.children.length > 50) {
                logArea.removeChild(logArea.firstChild);
            }
        }
        
        function clearLog() {
            var logArea = document.getElementById('logArea');
            logArea.innerHTML = '<div class="log-entry log-info">Log cleared. System ready.</div>';
        }
        
        function updateCurrentDisplay(x, y, w, h) {
            document.getElementById('currentSettings').textContent = 'Current: X=' + x + ', Y=' + y + ', Width=' + w + ', Height=' + h;
        }
        
        function applySettings() {
            var x = document.getElementById('xOffset').value;
            var y = document.getElementById('yOffset').value;
            var w = document.getElementById('width').value;
            var h = document.getElementById('height').value;
            
            // Width가 최소값보다 작으면 강제로 480으로 설정
            if (w < 480) {
                w = 480;
                document.getElementById('width').value = 480;
                addLog('Width adjusted to minimum value: 480', 'info');
            }
            
            var url = '/setROI?x=' + x + '&y=' + y + '&w=' + w + '&h=' + h;
            var streamImg = document.getElementById('stream');
            var applyBtn = document.getElementById('applyButton');
            
            // 버튼 비활성화
            applyBtn.disabled = true;
            applyBtn.textContent = 'Applying...';
            
            addLog('Sending ROI settings: ' + w + 'x' + h + ' @ (' + x + ', ' + y + ')', 'info');
            
            fetch(url)
                .then(response => {
                    addLog('Server response: ' + response.status, response.ok ? 'success' : 'error');
                    return response.text();
                })
                .then(data => {
                    document.getElementById('roiInfo').textContent = 'ROI: ' + w + 'x' + h + ' @ (' + x + ', ' + y + ')';
                    updateCurrentDisplay(x, y, w, h);
                    // 현재 값 업데이트
                    roi_offset_x = parseInt(x);
                    roi_offset_y = parseInt(y);
                    roi_width = parseInt(w);
                    roi_height = parseInt(h);
                    addLog('Settings applied: ' + data, 'success');
                    applyBtn.disabled = false;
                    applyBtn.textContent = 'Apply';
                })
                .catch(error => {
                    addLog('Error: ' + error.message, 'error');
                    applyBtn.disabled = false;
                    applyBtn.textContent = 'Apply';
                });
        }
        
        function setDefault() {
            document.getElementById('xOffset').value = 368;
            document.getElementById('yOffset').value = 380;
            document.getElementById('width').value = 544;
            document.getElementById('height').value = 264;
            addLog('Settings reset to default values', 'info');
            applySettings();
        }
        
        // 감지 결과 업데이트
        function updateDetection() {
            fetch('/get_detection')
                .then(response => response.json())
                .then(data => {
                    // 프레임 카운터 업데이트
                    if (data.frame_count !== undefined) {
                        document.getElementById('frameCounter').textContent = '(' + data.frame_count + ')';
                    }
                    
                    if (data.found && data.detections) {
                        // 감지 정보 표시
                        var scores = data.detections.map(d => (d.score * 100).toFixed(1) + '%').join(', ');
                        document.getElementById('detectionResult').innerHTML = 
                            'Golf ball detected! Scores: ' + scores;
                        
                        // 감지 개수 표시
                        if (data.detections.length > 1) {
                            addLog('=== ' + data.detections.length + ' balls detected ===', 'success');
                        }
                        
                        // 로그에 각 감지 결과 추가
                        data.detections.forEach((det, index) => {
                            addLog('Position: (' + 
                                   det.x.toFixed(0) + ', ' + 
                                   det.y.toFixed(0) + ') - ' +
                                   (det.score * 100).toFixed(1) + '% Height=' + 
                                   det.height.toFixed(0) + ', Width=' +
                                   det.width.toFixed(0), 'success');
                        });
                    } else if (data.found) {
                        // 기존 형식 지원 (호환성)
                        document.getElementById('detectionResult').innerHTML = 
                            'Golf ball detected! Score: ' + (data.score * 100).toFixed(1) + '%';
                        addLog('Position: (' + 
                               (data.x * roi_width).toFixed(0) + ', ' + 
                               (data.y * roi_height).toFixed(0) + ') - ' +
                               (data.score * 100).toFixed(1) + '%', 'success');
                    } else {
                        document.getElementById('detectionResult').innerHTML = 'No golf ball detected';
                    }
                })
                .catch(error => {
                    console.error('Detection update error:', error);
                });
        }
        
        // 이미지 새로고침 함수
        function refreshImage() {
            var img = document.getElementById('stream');
            img.src = '/image.jpg?t=' + new Date().getTime();
        }
        
        // 이미지 저장 함수
        function saveImage() {
            // 현재 시간으로 파일명 생성
            var timestamp = new Date();
            var filename = 'golf_ball_' + 
                          timestamp.getFullYear() + 
                          ('0' + (timestamp.getMonth() + 1)).slice(-2) +
                          ('0' + timestamp.getDate()).slice(-2) + '_' +
                          ('0' + timestamp.getHours()).slice(-2) +
                          ('0' + timestamp.getMinutes()).slice(-2) +
                          ('0' + timestamp.getSeconds()).slice(-2) + '.jpg';
            
            // 원본 이미지 다운로드 (박스 없는 이미지)
            fetch('/original.jpg')
                .then(response => response.blob())
                .then(blob => {
                    var url = window.URL.createObjectURL(blob);
                    var a = document.createElement('a');
                    a.href = url;
                    a.download = filename;
                    document.body.appendChild(a);
                    a.click();
                    window.URL.revokeObjectURL(url);
                    document.body.removeChild(a);
                    addLog('Image saved: ' + filename + ' (original without boxes)', 'success');
                })
                .catch(error => {
                    addLog('Failed to save image: ' + error, 'error');
                });
        }
        
        // 페이지 로드시 초기화
        window.onload = function() {
            addLog('Web interface loaded', 'success');
            // 감지 결과를 주기적으로 업데이트 (500ms마다)
            detection_interval = setInterval(updateDetection, 500);
            // 이미지도 주기적으로 새로고침 (200ms마다)
            setInterval(refreshImage, 200);
        };
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

// 원본 이미지 버퍼 (박스 없는 이미지)
static uint8_t *g_original_buffer = NULL;
static size_t g_original_size = 0;

// 감지 결과 및 ROI 설정
static bool detection_found = false;
static float detection_score = 0.0f;
static float detection_x = 0.0f;
static float detection_y = 0.0f;

// 모든 감지 결과 저장 (최대 10개)
static detection_info all_detections[10];
static int detection_count = 0;

// Live Stream 카운터
static uint32_t frame_counter = 0;

// 이미지 버퍼 설정 함수
void set_image_buffer(uint8_t *buffer, size_t size) {
    if (g_image_buffer != NULL) {
        free(g_image_buffer);
    }
    g_image_buffer = buffer;
    g_image_size = size;
    frame_counter++;  // 프레임 카운터 증가
}

// 원본 이미지 버퍼 설정 함수
void set_original_buffer(uint8_t *buffer, size_t size) {
    if (g_original_buffer != NULL) {
        free(g_original_buffer);
    }
    g_original_buffer = buffer;
    g_original_size = size;
}

// 감지 결과 설정 함수
void set_detection_result(bool found, float score, float x, float y) {
    detection_found = found;
    detection_score = score;
    detection_x = x;
    detection_y = y;
}

// 모든 감지 결과 설정
void set_all_detections(const detection_info* detections, int count) {
    detection_count = (count > 10) ? 10 : count;
    for (int i = 0; i < detection_count; i++) {
        all_detections[i] = detections[i];
    }
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

// 원본 이미지 전송 핸들러 (박스 없는 이미지)
static esp_err_t original_handler(httpd_req_t *req)
{
    if (g_original_buffer == NULL || g_original_size == 0) {
        ESP_LOGE(TAG, "No original image available");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    // HTTP 헤더 설정
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    char content_length[16];
    snprintf(content_length, sizeof(content_length), "%d", g_original_size);
    httpd_resp_set_hdr(req, "Content-Length", content_length);
    
    // 이미지 전송 - 청크 단위로
    size_t sent = 0;
    size_t chunk_size = 4096;
    while (sent < g_original_size) {
        size_t to_send = ((g_original_size - sent) > chunk_size) ? chunk_size : (g_original_size - sent);
        if (httpd_resp_send_chunk(req, (const char *)(g_original_buffer + sent), to_send) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send original chunk");
            return ESP_FAIL;
        }
        sent += to_send;
    }
    
    // 청크 전송 종료
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

// 스트림 핸들러 추가
static esp_err_t stream_handler(httpd_req_t *req)
{
    char part_buf[256];
    httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    while (true) {
        if (g_image_buffer == NULL || g_image_size == 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        // MJPEG 프레임 헤더
        size_t hlen = snprintf((char *)part_buf, 64, "--frame\r\nContent-Type: image/jpeg\r\n\r\n");
        if (httpd_resp_send_chunk(req, (const char *)part_buf, hlen) != ESP_OK) {
            break;
        }
        
        // 이미지 데이터 전송
        if (httpd_resp_send_chunk(req, (const char *)g_image_buffer, g_image_size) != ESP_OK) {
            break;
        }
        
        // 프레임 바운더리
        if (httpd_resp_send_chunk(req, "\r\n", 2) != ESP_OK) {
            break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 FPS
    }
    
    return ESP_OK;
}

// 버전 정보 핸들러
static esp_err_t version_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/plain");
    return httpd_resp_send(req, VERSION_STRING, strlen(VERSION_STRING));
}

// ROI 설정 핸들러
static esp_err_t setROI_handler(httpd_req_t *req)
{
    char query[256];
    char value[32];
    
    // URL 쿼리 파라미터 가져오기
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        // X offset
        if (httpd_query_key_value(query, "x", value, sizeof(value)) == ESP_OK) {
            int x = atoi(value);
            if (x >= 0 && x <= 800) {
                roi_offset_x = x;
            }
        }
        
        // Y offset
        if (httpd_query_key_value(query, "y", value, sizeof(value)) == ESP_OK) {
            int y = atoi(value);
            if (y >= 0 && y <= 864) {
                roi_offset_y = y;
            }
        }
        
        // Width
        if (httpd_query_key_value(query, "w", value, sizeof(value)) == ESP_OK) {
            int w = atoi(value);
            if (w >= 64 && w <= 800) {
                roi_width = w;
            }
        }
        
        // Height
        if (httpd_query_key_value(query, "h", value, sizeof(value)) == ESP_OK) {
            int h = atoi(value);
            if (h >= 64 && h <= 400) {
                roi_height = h;
            }
        }
        
        // 카메라 센서에 ROI 적용
        sensor_t *s = esp_camera_sensor_get();
        if (s) {
            roi_enabled = true;
            set_hw_roi_fixed(s);
            ESP_LOGI(TAG, "New ROI settings applied: %dx%d @ (%d, %d)", 
                     roi_width, roi_height, roi_offset_x, roi_offset_y);
            
            // ROI 적용 후 버퍼 플러시
            vTaskDelay(pdMS_TO_TICKS(200));
            for (int i = 0; i < 3; i++) {
                camera_fb_t *fb = esp_camera_fb_get();
                if (fb) {
                    esp_camera_fb_return(fb);
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
            }
        }
    }
    
    // 응답
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    char response[256];
    snprintf(response, sizeof(response), "OK - ROI set to %dx%d @ (%d, %d)", 
             roi_width, roi_height, roi_offset_x, roi_offset_y);
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// 감지 결과 반환 핸들러
static esp_err_t get_detection_handler(httpd_req_t *req)
{
    char response[1024];
    
    if (detection_count > 0) {
        // 모든 감지 결과를 JSON 배열로 전송
        int offset = snprintf(response, sizeof(response), 
                             "{\"found\":true,\"frame_count\":%lu,\"detections\":[", frame_counter);
        
        for (int i = 0; i < detection_count && offset < sizeof(response) - 100; i++) {
            if (i > 0) offset += snprintf(response + offset, sizeof(response) - offset, ",");
            offset += snprintf(response + offset, sizeof(response) - offset,
                              "{\"score\":%.3f,\"x\":%.1f,\"y\":%.1f,\"height\":%.1f,\"width\":%.1f}",
                              all_detections[i].score, all_detections[i].x, 
                              all_detections[i].y, all_detections[i].height, all_detections[i].width);
        }
        offset += snprintf(response + offset, sizeof(response) - offset, "]}");
    } else {
        snprintf(response, sizeof(response), "{\"found\":false,\"frame_count\":%lu}", frame_counter);
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, response, strlen(response));
}

// HTTP 서버 시작
static esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_open_sockets = 4;  // 다중 연결 허용
    config.recv_wait_timeout = 30;  // 수신 대기 시간 증가
    config.send_wait_timeout = 30;  // 전송 대기 시간 증가
    config.keep_alive_enable = true;  // Keep-alive 활성화
    config.keep_alive_idle = 5;      // Keep-alive 유휴 시간
    config.keep_alive_interval = 5;   // Keep-alive 간격
    config.keep_alive_count = 3;      // Keep-alive 재시도 횟수
    
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
        
        httpd_uri_t stream_uri = {
            .uri       = "/stream",
            .method    = HTTP_GET,
            .handler   = stream_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
        
        httpd_uri_t setROI_uri = {
            .uri       = "/setROI",
            .method    = HTTP_GET,
            .handler   = setROI_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &setROI_uri);
        
        httpd_uri_t get_detection_uri = {
            .uri       = "/get_detection",
            .method    = HTTP_GET,
            .handler   = get_detection_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_detection_uri);
        
        httpd_uri_t original_uri = {
            .uri       = "/original.jpg",
            .method    = HTTP_GET,
            .handler   = original_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &original_uri);
        
        ESP_LOGI(TAG, "Server Started with stream support");
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
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    
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