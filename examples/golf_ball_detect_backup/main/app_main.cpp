#include "cat_detect.hpp"
#include "golf_ball_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_log.h"
#include "bsp/esp-bsp.h"

extern const uint8_t cat_jpg_start[] asm("_binary_cat_jpg_start");
extern const uint8_t cat_jpg_end[] asm("_binary_cat_jpg_end");
extern const uint8_t golf_ball_00200_jpg_start[] asm("_binary_golf_ball_00200_jpg_start");
extern const uint8_t golf_ball_00200_jpg_end[] asm("_binary_golf_ball_00200_jpg_end");
extern const uint8_t golf_ball_00208_jpg_start[] asm("_binary_golf_ball_00208_jpg_start");
extern const uint8_t golf_ball_00208_jpg_end[] asm("_binary_golf_ball_00208_jpg_end");

const char *TAG = "detect_demo";

extern "C" void app_main(void)
{
#if CONFIG_CAT_DETECT_MODEL_IN_SDCARD || CONFIG_GOLF_BALL_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

    // First: Detect cats in cat.jpg
    ESP_LOGI(TAG, "=== Cat Detection ===");
    dl::image::jpeg_img_t cat_jpeg = {.data = (void *)cat_jpg_start, .data_len = (size_t)(cat_jpg_end - cat_jpg_start)};
    auto cat_img = dl::image::sw_decode_jpeg(cat_jpeg, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    
    if (cat_img.data != nullptr) {
        CatDetect *cat_detect = new CatDetect();
        auto &cat_results = cat_detect->run(cat_img);
        
        ESP_LOGI(TAG, "Found %d cats:", cat_results.size());
        for (const auto &res : cat_results) {
            ESP_LOGI(TAG,
                     "[Cat - score: %.2f%%, x1: %d, y1: %d, x2: %d, y2: %d]",
                     res.score * 100,
                     res.box[0],
                     res.box[1],
                     res.box[2],
                     res.box[3]);
        }
        delete cat_detect;
        heap_caps_free(cat_img.data);
    }
    
    // Second: Detect golf balls in golf_ball_00200.jpg
    ESP_LOGI(TAG, "\n=== Golf Ball Detection (Image 1) ===");
    dl::image::jpeg_img_t golf1_jpeg = {.data = (void *)golf_ball_00200_jpg_start, 
                                        .data_len = (size_t)(golf_ball_00200_jpg_end - golf_ball_00200_jpg_start)};
    auto golf1_img = dl::image::sw_decode_jpeg(golf1_jpeg, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    
    if (golf1_img.data != nullptr) {
        GolfBallDetect *golf_detect = new GolfBallDetect();
        auto &golf1_results = golf_detect->run(golf1_img);
        
        ESP_LOGI(TAG, "Found %d golf balls:", golf1_results.size());
        for (const auto &res : golf1_results) {
            ESP_LOGI(TAG,
                     "[Golf ball - score: %.2f%%, x1: %d, y1: %d, x2: %d, y2: %d]",
                     res.score * 100,
                     res.box[0],
                     res.box[1],
                     res.box[2],
                     res.box[3]);
        }
        delete golf_detect;
        heap_caps_free(golf1_img.data);
    }
    
    // Third: Detect golf balls in golf_ball_00208.jpg
    ESP_LOGI(TAG, "\n=== Golf Ball Detection (Image 2) ===");
    dl::image::jpeg_img_t golf2_jpeg = {.data = (void *)golf_ball_00208_jpg_start, 
                                        .data_len = (size_t)(golf_ball_00208_jpg_end - golf_ball_00208_jpg_start)};
    auto golf2_img = dl::image::sw_decode_jpeg(golf2_jpeg, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    
    if (golf2_img.data != nullptr) {
        GolfBallDetect *golf_detect = new GolfBallDetect();
        auto &golf2_results = golf_detect->run(golf2_img);
        
        ESP_LOGI(TAG, "Found %d golf balls:", golf2_results.size());
        for (const auto &res : golf2_results) {
            ESP_LOGI(TAG,
                     "[Golf ball - score: %.2f%%, x1: %d, y1: %d, x2: %d, y2: %d]",
                     res.score * 100,
                     res.box[0],
                     res.box[1],
                     res.box[2],
                     res.box[3]);
        }
        delete golf_detect;
        heap_caps_free(golf2_img.data);
    }

#if CONFIG_CAT_DETECT_MODEL_IN_SDCARD || CONFIG_GOLF_BALL_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif
}
