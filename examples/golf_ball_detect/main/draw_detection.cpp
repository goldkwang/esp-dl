#include "draw_detection.hpp"
#include "esp_log.h"
#include "dl_image.hpp"
#include "dl_image_jpeg.hpp"
#include <list>
#include <stdio.h>
#include <string.h>

void draw_detection_boxes(dl::image::img_t &img, const std::list<dl::detect::result_t> &results) {
    const char *TAG = "draw_detection";
    
    // Define box color (white)
    uint8_t r = 255, g = 255, b = 255;
    
    for (const auto &res : results) {
        int x1 = res.box[0];
        int y1 = res.box[1];
        int x2 = res.box[2];
        int y2 = res.box[3];
        
        ESP_LOGI(TAG, "Drawing box: (%d,%d) to (%d,%d)", x1, y1, x2, y2);
        
        // Draw horizontal lines
        for (int x = x1; x <= x2; x++) {
            // Top line
            if (y1 >= 0 && y1 < img.height && x >= 0 && x < img.width) {
                int idx = (y1 * img.width + x) * 3;
                ((uint8_t*)img.data)[idx] = r;
                ((uint8_t*)img.data)[idx + 1] = g;
                ((uint8_t*)img.data)[idx + 2] = b;
            }
            // Bottom line
            if (y2 >= 0 && y2 < img.height && x >= 0 && x < img.width) {
                int idx = (y2 * img.width + x) * 3;
                ((uint8_t*)img.data)[idx] = r;
                ((uint8_t*)img.data)[idx + 1] = g;
                ((uint8_t*)img.data)[idx + 2] = b;
            }
        }
        
        // Draw vertical lines
        for (int y = y1; y <= y2; y++) {
            // Left line
            if (y >= 0 && y < img.height && x1 >= 0 && x1 < img.width) {
                int idx = (y * img.width + x1) * 3;
                ((uint8_t*)img.data)[idx] = r;
                ((uint8_t*)img.data)[idx + 1] = g;
                ((uint8_t*)img.data)[idx + 2] = b;
            }
            // Right line
            if (y >= 0 && y < img.height && x2 >= 0 && x2 < img.width) {
                int idx = (y * img.width + x2) * 3;
                ((uint8_t*)img.data)[idx] = r;
                ((uint8_t*)img.data)[idx + 1] = g;
                ((uint8_t*)img.data)[idx + 2] = b;
            }
        }
    }
}

bool save_image_with_boxes_to_sdcard(dl::image::img_t &img, const std::list<dl::detect::result_t> &results, const char* filename) {
    const char *TAG = "save_detection";
    
    // Draw boxes on image
    draw_detection_boxes(img, results);
    
    // Encode to JPEG
    uint8_t quality = 85;
    dl::image::jpeg_img_t jpeg_result = dl::image::sw_encode_jpeg(img, 0, quality);
    
    if (!jpeg_result.data) {
        ESP_LOGE(TAG, "Failed to encode JPEG");
        return false;
    }
    
    // Create full file path
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "/sdcard/%s", filename);
    
    // Save to SD card
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", filepath);
        heap_caps_free(jpeg_result.data);
        return false;
    }
    
    size_t written = fwrite(jpeg_result.data, 1, jpeg_result.data_len, f);
    fclose(f);
    heap_caps_free(jpeg_result.data);
    
    if (written != jpeg_result.data_len) {
        ESP_LOGE(TAG, "Failed to write complete file");
        return false;
    }
    
    ESP_LOGI(TAG, "Detection result saved to: %s (%d bytes)", filepath, jpeg_result.data_len);
    return true;
}