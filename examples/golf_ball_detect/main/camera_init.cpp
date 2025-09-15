#include "camera_init.hpp"
#include "esp_log.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "camera_init";

// ROI configuration variables - 전체화면 고정
bool roi_enabled = false;  // 항상 전체화면
int roi_offset_x = 0;    
int roi_offset_y = 0;  
int roi_width = 1280;       
int roi_height = 1024;

// Camera configuration
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SXGA,     // 1280x1024 전체 화면
    .jpeg_quality = 12,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST
};

// Set hardware ROI
void set_hw_roi_fixed(sensor_t *sensor) {
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor null");
        return;
    }
    
    if (!roi_enabled) {
        ESP_LOGI(TAG, "ROI disabled - using full frame");
        // 전체 프레임으로 설정
        sensor->set_framesize(sensor, FRAMESIZE_SXGA);
        return;
    }
    
    ESP_LOGI(TAG, "Setting HW ROI: %dx%d @ (%d, %d)", 
             roi_width, roi_height, roi_offset_x, roi_offset_y);
    
    // Bank selection to DSP
    sensor->set_reg(sensor, BANK_SEL, 0xff, BANK_DSP);
    
    // ROI offsets (divide by 4 for register values)
    uint16_t hoff = roi_offset_x / 4;
    uint16_t voff = roi_offset_y / 4;
    
    // Set horizontal offset
    sensor->set_reg(sensor, REG_XOFFL, 0xff, hoff & 0xff);
    
    // Set vertical offset  
    sensor->set_reg(sensor, REG_YOFFL, 0xff, voff & 0xff);
    
    // Set ROI size (divide by 4)
    uint16_t hsize = roi_width / 4;
    uint16_t vsize = roi_height / 4;
    
    sensor->set_reg(sensor, REG_HSIZE, 0xff, hsize & 0xff);
    sensor->set_reg(sensor, REG_VSIZE, 0xff, vsize & 0xff);
    
    // VHYX register - write once with all bits combined
    uint8_t vhyx_val = 0;
    vhyx_val |= (hoff >> 8) & 0x03;     // bits 1:0 - horizontal offset high
    vhyx_val |= (hsize >> 6) & 0x0c;    // bits 3:2 - horizontal size high  
    vhyx_val |= (vsize >> 4) & 0x30;    // bits 5:4 - vertical size high
    vhyx_val |= ((voff >> 2) & 0x40) | ((voff >> 4) & 0x80);  // bits 7:6 - vertical offset high
    sensor->set_reg(sensor, REG_VHYX, 0xff, vhyx_val);
    
    // Zoom window size
    sensor->set_reg(sensor, REG_ZMOW, 0xff, (roi_width / 4) & 0xff);
    sensor->set_reg(sensor, REG_ZMOH, 0xff, (roi_height / 4) & 0xff);
    sensor->set_reg(sensor, REG_ZMHH, 0xff, 
                    (((roi_height / 4) >> 8) & 0x0f) | 
                    (((roi_width / 4) >> 4) & 0xf0));
    
    ESP_LOGI(TAG, "HW ROI configured successfully");
    ESP_LOGI(TAG, "Registers - HOFF:%d VOFF:%d HSIZE:%d VSIZE:%d VHYX:0x%02X", 
             hoff, voff, hsize, vsize, vhyx_val);
}

// Initialize camera
esp_err_t camera_init() {
    ESP_LOGI(TAG, "Initializing camera...");
    
    // Camera power stabilization
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Retry
        err = esp_camera_init(&camera_config);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Camera reinit also failed with error 0x%x", err);
            return err;
        }
    }
    
    // Wait for camera stabilization
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Get sensor handle
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        // Configure sensor
        s->set_framesize(s, FRAMESIZE_SXGA);  // 1280x1024
        vTaskDelay(pdMS_TO_TICKS(100));
        s->set_quality(s, 10);  // High quality
        s->set_brightness(s, 1); // Slightly increase brightness
        s->set_contrast(s, 1);   // Slightly increase contrast
        s->set_saturation(s, -1); // Decrease saturation (for white golf ball)
        s->set_sharpness(s, 1);  // Increase sharpness
        s->set_denoise(s, 0);    // Denoise OFF
        s->set_ae_level(s, 0);   // Auto exposure default
        s->set_aec2(s, 0);       // AEC DSP disabled
        s->set_exposure_ctrl(s, 1); // Exposure control enabled
        s->set_aec_value(s, 300);   // Fixed exposure value
        s->set_gainceiling(s, (gainceiling_t)GAINCEILING_4X); // Gain limit
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if (s->id.PID == OV2640_PID) {
            // OV2640 specific settings - 180 degree rotation
            s->set_vflip(s, 1);    // Vertical flip
            s->set_hmirror(s, 1);  // Horizontal mirror
            
            // Wait for camera stabilization
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // Warm up
            for (int i = 0; i < 3; i++) {
                camera_fb_t *fb = esp_camera_fb_get();
                if (fb) {
                    esp_camera_fb_return(fb);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }
            
            // 초기에는 전체화면 모드
            roi_enabled = false;
            
            ESP_LOGI(TAG, "OV2640 camera initialized - Full Frame Mode");
        }
    } else {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}

// Get camera frame buffer
camera_fb_t* camera_get_fb() {
    return esp_camera_fb_get();
}

// Return camera frame buffer
void camera_return_fb(camera_fb_t *fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}