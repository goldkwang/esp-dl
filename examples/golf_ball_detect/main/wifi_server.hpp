#ifndef WIFI_SERVER_HPP
#define WIFI_SERVER_HPP

#include <stddef.h>
#include <stdint.h>

// WiFi 서버 시작
void start_wifi_server(void);

// 이미지 버퍼 설정
void set_image_buffer(uint8_t *buffer, size_t size);

// 원본 이미지 버퍼 설정 (박스 없는 이미지)
void set_original_buffer(uint8_t *buffer, size_t size);

// 감지 결과 구조체
struct detection_info {
    float score;
    float x;
    float y;
    float height;
    float width;
};

// 감지 결과 설정
void set_detection_result(bool found, float score, float x, float y);

// 모든 감지 결과 설정 (최대 10개)
void set_all_detections(const detection_info* detections, int count);

#endif // WIFI_SERVER_HPP