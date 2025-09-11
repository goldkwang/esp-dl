#ifndef WIFI_SERVER_HPP
#define WIFI_SERVER_HPP

#include <stddef.h>
#include <stdint.h>

// WiFi 서버 시작
void start_wifi_server(void);

// 이미지 버퍼 설정
void set_image_buffer(uint8_t *buffer, size_t size);

#endif // WIFI_SERVER_HPP