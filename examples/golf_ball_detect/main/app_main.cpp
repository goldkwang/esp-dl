#include "cat_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "draw_detection.hpp"
#include "wifi_server.hpp"
#include <string.h>

extern const uint8_t cat_jpg_start[] asm("_binary_cat_jpg_start");
extern const uint8_t cat_jpg_end[] asm("_binary_cat_jpg_end");
const char *TAG = "cat_detect";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "\n\n========================================");
    ESP_LOGI(TAG, "   GOLF BALL DETECTION %s START", VERSION_STRING);
    ESP_LOGI(TAG, "========================================\n");
    dl::image::jpeg_img_t jpeg_img = {.data = (void *)cat_jpg_start, .data_len = (size_t)(cat_jpg_end - cat_jpg_start)};
    auto img = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);

    ESP_LOGI(TAG, "Image decoded - Width: %d, Height: %d", img.width, img.height);
    ESP_LOGI(TAG, "Creating CatDetect instance with golf ball model...");
    
    CatDetect *detect = new CatDetect();
    auto &detect_results = detect->run(img);
    
    ESP_LOGI(TAG, "Detection complete. Found %d object(s)", detect_results.size());
    for (const auto &res : detect_results) {
        ESP_LOGI(TAG,
                 "[category: %d, score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
                 res.category,
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }
    
    // Draw detection boxes on image
    ESP_LOGI(TAG, "Drawing detection boxes on image...");
    draw_detection_boxes(img, detect_results);
    
    // Convert to JPEG and set for WiFi display
    ESP_LOGI(TAG, "Encoding image to JPEG...");
    dl::image::jpeg_img_t jpeg_result = dl::image::sw_encode_jpeg(img, 0, 85);
    
    if (jpeg_result.data) {
        ESP_LOGI(TAG, "Setting image buffer for web server (%d bytes)", jpeg_result.data_len);
        // Copy data since set_image_buffer will own it
        uint8_t *buffer_copy = (uint8_t*)malloc(jpeg_result.data_len);
        if (buffer_copy) {
            memcpy(buffer_copy, jpeg_result.data, jpeg_result.data_len);
            set_image_buffer(buffer_copy, jpeg_result.data_len);
        }
        heap_caps_free(jpeg_result.data);
    }
    
    delete detect;
    heap_caps_free(img.data);


    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "   GOLF BALL DETECTION %s COMPLETE", VERSION_STRING);
    ESP_LOGI(TAG, "========================================\n\n");
    
    // Wait a bit before starting WiFi
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Start WiFi server
    ESP_LOGI(TAG, "Starting WiFi server...");
    start_wifi_server();
    ESP_LOGI(TAG, "System ready. Access web interface at http://192.168.0.11");
}