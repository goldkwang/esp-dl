#include "cat_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "draw_detection.hpp"
#include "wifi_server.hpp"
#include "camera_init.hpp"
#include "img_converters.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

const char *TAG = "golf_ball_detect";

// 감지 태스크
static void detection_task(void *pvParameters) {
    CatDetect *detect = new CatDetect();
    ESP_LOGI(TAG, "Detection task started");
    
    while (1) {
        camera_fb_t *fb = camera_get_fb();
        if (fb) {
            // JPEG를 RGB888로 변환
            uint8_t *rgb_buf = (uint8_t *)heap_caps_malloc(roi_width * roi_height * 3, MALLOC_CAP_SPIRAM);
            if (rgb_buf) {
                // RGB565로 먼저 변환
                uint8_t *rgb565_buf = (uint8_t *)heap_caps_malloc(roi_width * roi_height * 2, MALLOC_CAP_SPIRAM);
                if (rgb565_buf) {
                    bool converted = jpg2rgb565(fb->buf, fb->len, rgb565_buf, JPG_SCALE_NONE);
                    if (converted) {
                        // RGB565를 RGB888로 변환
                        for (int i = 0; i < roi_width * roi_height; i++) {
                            uint16_t pixel = ((uint16_t*)rgb565_buf)[i];
                            int idx = i * 3;
                            uint8_t r = ((pixel >> 11) & 0x1F);
                            uint8_t g = ((pixel >> 5) & 0x3F);
                            uint8_t b = (pixel & 0x1F);
                            rgb_buf[idx] = (r << 3) | (r >> 2);
                            rgb_buf[idx + 1] = (g << 2) | (g >> 4);
                            rgb_buf[idx + 2] = (b << 3) | (b >> 2);
                        }
                        
                        // dl::image::img_t 구조체 생성
                        dl::image::img_t img = {
                            .data = rgb_buf,
                            .width = (uint16_t)roi_width,
                            .height = (uint16_t)roi_height,
                            .pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888
                        };
                        
                        // 검출 수행
                        auto &detect_results = detect->run(img);
                        
                        // 검출 결과 설정
                        if (!detect_results.empty()) {
                            const auto &res = detect_results.front();
                            float score = res.score;
                            float x = (res.box[0] + res.box[2]) / 2.0f / roi_width;
                            float y = (res.box[1] + res.box[3]) / 2.0f / roi_height;
                            set_detection_result(true, score, x, y);
                            float box_height = res.box[3] - res.box[1];
                            ESP_LOGI(TAG, "Golf ball detected: %.2f%% at (%.1f, %.1f) Height=%.1f",
                                     score * 100, x * roi_width, y * roi_height, box_height);
                        } else {
                            set_detection_result(false, 0, 0, 0);
                        }
                        
                        // 검출 박스 그리기
                        draw_detection_boxes(img, detect_results);
                        
                        // JPEG로 다시 인코딩
                        dl::image::jpeg_img_t jpeg_result = dl::image::sw_encode_jpeg(img, 0, 85);
                        if (jpeg_result.data) {
                            uint8_t *buffer_copy = (uint8_t*)malloc(jpeg_result.data_len);
                            if (buffer_copy) {
                                memcpy(buffer_copy, jpeg_result.data, jpeg_result.data_len);
                                set_image_buffer(buffer_copy, jpeg_result.data_len);
                            }
                            heap_caps_free(jpeg_result.data);
                        }
                    }
                    heap_caps_free(rgb565_buf);
                }
                heap_caps_free(rgb_buf);
            }
            camera_return_fb(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 더 빠른 처리 시도
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "\n\n========================================");
    ESP_LOGI(TAG, "   GOLF BALL DETECTION %s START", VERSION_STRING);
    ESP_LOGI(TAG, "========================================\n");
    
    // 카메라 초기화
    ESP_LOGI(TAG, "Initializing camera...");
    esp_err_t err = camera_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed");
        return;
    }
    ESP_LOGI(TAG, "Camera initialized successfully");
    
    // WiFi 서버 시작
    ESP_LOGI(TAG, "Starting WiFi server...");
    start_wifi_server();
    ESP_LOGI(TAG, "System ready. Access web interface at http://192.168.0.11");
    
    // 감지 태스크 생성
    xTaskCreate(detection_task, "detection_task", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "Detection task created");
    
    // 메인 태스크는 유지
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}