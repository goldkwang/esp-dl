#pragma once

#include "esp_camera.h"
#include "esp_err.h"

// Camera pins for ESP-WROVER board
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5
#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

// OV2640 sensor ID
#define OV2640_PID  0x26

// OV2640 Gain Ceiling values
#define GAINCEILING_2X      0x00
#define GAINCEILING_4X      0x01
#define GAINCEILING_8X      0x02
#define GAINCEILING_16X     0x03
#define GAINCEILING_32X     0x04
#define GAINCEILING_64X     0x05
#define GAINCEILING_128X    0x06

// OV2640 register definitions
#define BANK_SEL        0xFF
#define BANK_DSP        0x00
#define BANK_SENSOR     0x01

// OV2640 ROI registers (DSP Bank)
#define REG_HSIZE       0x51
#define REG_VSIZE       0x52
#define REG_XOFFL       0x53
#define REG_YOFFL       0x54
#define REG_VHYX        0x55
#define REG_TEST        0x57
#define REG_ZMOW        0x5A
#define REG_ZMOH        0x5B
#define REG_ZMHH        0x5C

// ROI configuration
extern bool roi_enabled;
extern int roi_offset_x;
extern int roi_offset_y;
extern int roi_width;
extern int roi_height;

// Camera initialization
esp_err_t camera_init();
void set_hw_roi_fixed(sensor_t *sensor);

// Get camera frame buffer
camera_fb_t* camera_get_fb();
void camera_return_fb(camera_fb_t *fb);