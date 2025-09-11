#pragma once

#include "dl_image.hpp"
#include "dl_detect_define.hpp"
#include <list>

void draw_detection_boxes(dl::image::img_t &img, const std::list<dl::detect::result_t> &results);
bool save_image_with_boxes_to_sdcard(dl::image::img_t &img, const std::list<dl::detect::result_t> &results, const char* filename);