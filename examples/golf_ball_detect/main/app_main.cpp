#include "cat_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "draw_detection.hpp"

extern const uint8_t cat_jpg_start[] asm("_binary_cat_jpg_start");
extern const uint8_t cat_jpg_end[] asm("_binary_cat_jpg_end");
const char *TAG = "cat_detect";

extern "C" void app_main(void)
{
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
    
    // Save image with detection boxes to SD card
    if (detect_results.size() > 0) {
        ESP_LOGI(TAG, "Saving detection result to SD card...");
        ESP_ERROR_CHECK(bsp_sdcard_mount());
        
        if (save_image_with_boxes_to_sdcard(img, detect_results, "cat_detection_result.jpg")) {
            ESP_LOGI(TAG, "Detection result saved successfully!");
        } else {
            ESP_LOGE(TAG, "Failed to save detection result");
        }
        
        ESP_ERROR_CHECK(bsp_sdcard_unmount());
    }
    
    delete detect;
    heap_caps_free(img.data);


    ESP_LOGI(TAG, "Cat Detection %s", VERSION_STRING);
}