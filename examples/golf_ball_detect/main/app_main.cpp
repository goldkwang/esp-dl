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
            // 원본 JPEG 이미지 저장 (박스 그리기 전)
            uint8_t *original_copy = (uint8_t*)malloc(fb->len);
            if (original_copy) {
                memcpy(original_copy, fb->buf, fb->len);
                set_original_buffer(original_copy, fb->len);
            }
            
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
                        ESP_LOGI(TAG, "Raw detection count: %d", detect_results.size());
                        
                        // 검출 결과 설정
                        if (!detect_results.empty()) {
                            // 첫 번째 감지만 기존 함수로 설정 (호환성 유지)
                            const auto &res = detect_results.front();
                            float score = res.score;
                            float x = (res.box[0] + res.box[2]) / 2.0f / roi_width;
                            float y = (res.box[1] + res.box[3]) / 2.0f / roi_height;
                            set_detection_result(true, score, x, y);
                            float box_height = res.box[3] - res.box[1];
                            ESP_LOGI(TAG, "Golf ball detected: %.2f%% at (%.1f, %.1f) Height=%.1f",
                                     score * 100, x * roi_width, y * roi_height, box_height);
                            
                            // 모든 감지 결과 저장 (width <= 160인 것만)
                            detection_info detections[10];
                            int count = 0;
                            for (const auto &det : detect_results) {
                                if (count >= 10) break;
                                float det_width = det.box[2] - det.box[0];
                                float det_height = det.box[3] - det.box[1];
                                
                                // width > 160이면 콘솔에만 로그 출력
                                if (det_width > 160) {
                                    ESP_LOGW(TAG, "Invalid detection - width too large: %.1f pixels (> 160), height=%.1f", 
                                             det_width, det_height);
                                    continue;  // 웹에는 전달하지 않음
                                }
                                
                                detections[count].score = det.score;
                                detections[count].x = (det.box[0] + det.box[2]) / 2.0f;
                                detections[count].y = (det.box[1] + det.box[3]) / 2.0f;
                                detections[count].height = det_height;
                                detections[count].width = det_width;
                                count++;
                            }
                            set_all_detections(detections, count);
                            ESP_LOGI(TAG, "Valid detections after filtering: %d", count);
                            
                            // 필터링 후 유효한 감지가 없으면 감지 상태도 false로 설정
                            if (count == 0) {
                                set_detection_result(false, 0, 0, 0);
                            }
                        } else {
                            set_detection_result(false, 0, 0, 0);
                            set_all_detections(nullptr, 0);
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