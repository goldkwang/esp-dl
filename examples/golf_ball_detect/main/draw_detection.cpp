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
    
    // Draw version on image - large solid rectangle with version
    const char* version = VERSION_STRING;
    int rect_x = 10;
    int rect_y = 10;
    int rect_width = 60;
    int rect_height = 25;
    
    // Draw black background rectangle
    for (int y = rect_y; y < rect_y + rect_height && y < img.height; y++) {
        for (int x = rect_x; x < rect_x + rect_width && x < img.width; x++) {
            int idx = (y * img.width + x) * 3;
            ((uint8_t*)img.data)[idx] = 0;       // R
            ((uint8_t*)img.data)[idx + 1] = 0;   // G
            ((uint8_t*)img.data)[idx + 2] = 0;   // B
        }
    }
    
    // Draw white border
    for (int x = rect_x; x < rect_x + rect_width && x < img.width; x++) {
        if (rect_y < img.height) {
            int idx = (rect_y * img.width + x) * 3;
            ((uint8_t*)img.data)[idx] = 255;
            ((uint8_t*)img.data)[idx + 1] = 255;
            ((uint8_t*)img.data)[idx + 2] = 255;
        }
        if (rect_y + rect_height - 1 < img.height) {
            int idx = ((rect_y + rect_height - 1) * img.width + x) * 3;
            ((uint8_t*)img.data)[idx] = 255;
            ((uint8_t*)img.data)[idx + 1] = 255;
            ((uint8_t*)img.data)[idx + 2] = 255;
        }
    }
    for (int y = rect_y; y < rect_y + rect_height && y < img.height; y++) {
        if (rect_x < img.width) {
            int idx = (y * img.width + rect_x) * 3;
            ((uint8_t*)img.data)[idx] = 255;
            ((uint8_t*)img.data)[idx + 1] = 255;
            ((uint8_t*)img.data)[idx + 2] = 255;
        }
        if (rect_x + rect_width - 1 < img.width) {
            int idx = (y * img.width + (rect_x + rect_width - 1)) * 3;
            ((uint8_t*)img.data)[idx] = 255;
            ((uint8_t*)img.data)[idx + 1] = 255;
            ((uint8_t*)img.data)[idx + 2] = 255;
        }
    }
    
    // Draw simple version indicator - filled circles for version number
    // v6 = 6 white dots in 2 rows
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 3; col++) {
            int dot_x = rect_x + 10 + col * 15;
            int dot_y = rect_y + 8 + row * 10;
            
            // Draw 3x3 white square for each dot
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int px = dot_x + dx;
                    int py = dot_y + dy;
                    if (px >= 0 && px < img.width && py >= 0 && py < img.height) {
                        int idx = (py * img.width + px) * 3;
                        ((uint8_t*)img.data)[idx] = 255;
                        ((uint8_t*)img.data)[idx + 1] = 255;
                        ((uint8_t*)img.data)[idx + 2] = 255;
                    }
                }
            }
        }
    }
    
    ESP_LOGI(TAG, "Version %s drawn on image", version);
    
    ESP_LOGI(TAG, "Total detection results: %d", results.size());
    
    int box_index = 0;
    for (const auto &res : results) {
        int x1 = res.box[0];
        int y1 = res.box[1];
        int x2 = res.box[2];
        int y2 = res.box[3];
        
        ESP_LOGI(TAG, "Box %d - Original coordinates: (%d,%d) to (%d,%d), score: %.2f", 
                 box_index++, x1, y1, x2, y2, res.score);
        
        // Calculate center and size
        int center_x = (x1 + x2) / 2;
        int center_y = (y1 + y2) / 2;
        int width = x2 - x1;
        int height = y2 - y1;
        
        // Use the smaller dimension to make a square
        int size = (width < height) ? width : height;
        // Increase box size by 10% to better cover the golf ball
        size = (int)(size * 1.1);
        int half_size = size / 2;
        
        // Recalculate square box coordinates
        x1 = center_x - half_size;
        y1 = center_y - half_size;
        x2 = center_x + half_size;
        y2 = center_y + half_size;
        
        // Ensure coordinates are within image bounds
        if (x1 < 0) x1 = 0;
        if (y1 < 0) y1 = 0;
        if (x2 >= img.width) x2 = img.width - 1;
        if (y2 >= img.height) y2 = img.height - 1;
        
        ESP_LOGI(TAG, "Drawing square box: (%d,%d) to (%d,%d), size: %dx%d", x1, y1, x2, y2, size, size);
        
        // Draw horizontal lines (2 pixels thick)
        for (int x = x1; x <= x2; x++) {
            // Top line
            for (int thickness = 0; thickness < 2; thickness++) {
                int y = y1 + thickness;
                if (y >= 0 && y < img.height && x >= 0 && x < img.width) {
                    int idx = (y * img.width + x) * 3;
                    ((uint8_t*)img.data)[idx] = r;
                    ((uint8_t*)img.data)[idx + 1] = g;
                    ((uint8_t*)img.data)[idx + 2] = b;
                }
            }
            // Bottom line
            for (int thickness = 0; thickness < 2; thickness++) {
                int y = y2 - thickness;
                if (y >= 0 && y < img.height && x >= 0 && x < img.width) {
                    int idx = (y * img.width + x) * 3;
                    ((uint8_t*)img.data)[idx] = r;
                    ((uint8_t*)img.data)[idx + 1] = g;
                    ((uint8_t*)img.data)[idx + 2] = b;
                }
            }
        }
        
        // Draw vertical lines (2 pixels thick)
        for (int y = y1; y <= y2; y++) {
            // Left line
            for (int thickness = 0; thickness < 2; thickness++) {
                int x = x1 + thickness;
                if (y >= 0 && y < img.height && x >= 0 && x < img.width) {
                    int idx = (y * img.width + x) * 3;
                    ((uint8_t*)img.data)[idx] = r;
                    ((uint8_t*)img.data)[idx + 1] = g;
                    ((uint8_t*)img.data)[idx + 2] = b;
                }
            }
            // Right line
            for (int thickness = 0; thickness < 2; thickness++) {
                int x = x2 - thickness;
                if (y >= 0 && y < img.height && x >= 0 && x < img.width) {
                    int idx = (y * img.width + x) * 3;
                    ((uint8_t*)img.data)[idx] = r;
                    ((uint8_t*)img.data)[idx + 1] = g;
                    ((uint8_t*)img.data)[idx + 2] = b;
                }
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