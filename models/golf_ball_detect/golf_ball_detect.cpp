#include "golf_ball_detect.hpp"
#include "esp_log.h"

#if CONFIG_GOLF_BALL_DETECT_MODEL_IN_FLASH_RODATA
extern const uint8_t golf_ball_detect_espdl[] asm("_binary_golf_ball_detect_espdl_start");
static const char *path = (const char *)golf_ball_detect_espdl;
#elif CONFIG_GOLF_BALL_DETECT_MODEL_IN_FLASH_PARTITION
static const char *path = "golf_det";
#else
#if !defined(CONFIG_BSP_SD_MOUNT_POINT)
#define CONFIG_BSP_SD_MOUNT_POINT "/sdcard"
#endif
#endif

namespace golf_ball_detect {
ESPDet::ESPDet(const char *model_name)
{
#if !CONFIG_GOLF_BALL_DETECT_MODEL_IN_SDCARD
    m_model =
        new dl::Model(path, model_name, static_cast<fbs::model_location_type_t>(CONFIG_GOLF_BALL_DETECT_MODEL_LOCATION));
#else
    char sd_path[256];
    snprintf(sd_path,
             sizeof(sd_path),
             "%s/%s/%s",
             CONFIG_BSP_SD_MOUNT_POINT,
             CONFIG_GOLF_BALL_DETECT_MODEL_SDCARD_DIR,
             model_name);
    m_model = new dl::Model(sd_path, static_cast<fbs::model_location_type_t>(CONFIG_GOLF_BALL_DETECT_MODEL_LOCATION));
#endif
    m_model->minimize();
#if CONFIG_IDF_TARGET_ESP32P4
    m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {255, 255, 255});
#else
    m_image_preprocessor = new dl::image::ImagePreprocessor(
        m_model, {0, 0, 0}, {255, 255, 255}, dl::image::DL_IMAGE_CAP_RGB565_BIG_ENDIAN);
#endif
    m_image_preprocessor->enable_letterbox({114, 114, 114});
    m_postprocessor = new dl::detect::ESPDetPostProcessor(
        m_model, m_image_preprocessor, 0.25, 0.7, 10, {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}});
}

} // namespace golf_ball_detect

GolfBallDetect::GolfBallDetect(model_type_t model_type)
{
    switch (model_type) {
    case GOLF_BALL_99_4_MAP:
#if CONFIG_FLASH_GOLF_BALL_99_4_MAP || CONFIG_GOLF_BALL_DETECT_MODEL_IN_SDCARD
        m_model = new golf_ball_detect::ESPDet("golf_ball_model.espdl");
#else
        ESP_LOGE("golf_ball_detect", "golf_ball_99_4_mAP is not selected in menuconfig.");
#endif
        break;
    }
}