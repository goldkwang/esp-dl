#pragma once
#include "dl_detect_base.hpp"
#include "dl_detect_espdet_postprocessor.hpp"

namespace golf_ball_detect {
class ESPDet : public dl::detect::DetectImpl {
public:
    ESPDet(const char *model_name);
};
} // namespace golf_ball_detect

class GolfBallDetect {
public:
    enum class model_type_t {
        GOLF_BALL_99_4_MAP
    };
    
    GolfBallDetect(model_type_t model_type = model_type_t::GOLF_BALL_99_4_MAP);
    ~GolfBallDetect() { delete m_model; }
    
    std::list<dl::detect::result_t> &run(dl::image::img_t &img)
    {
        return m_model->run(img);
    }

private:
    golf_ball_detect::ESPDet *m_model;
};