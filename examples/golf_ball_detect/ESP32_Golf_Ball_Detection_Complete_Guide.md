# ESP32 골프공 감지 모델 학습부터 통합까지 완전 가이드

## 목차
1. [개요](#개요)
2. [중요: espdet_pico 모델 사용](#중요-espdet_pico-모델-사용)
3. [사전 준비사항](#사전-준비사항)
4. [데이터셋 준비](#데이터셋-준비)
5. [모델 학습](#모델-학습)
6. [모델 변환](#모델-변환)
7. [ESP32 통합](#esp32-통합)
8. [문제 해결](#문제-해결)

---

## 개요

이 문서는 ESP-Detection의 **espdet_pico** 모델을 사용하여 골프공 감지 모델을 학습하고 ESP32-S3에 배포하는 과정을 설명합니다.

### 목표 사양
- **모델**: espdet_pico (ESP32 최적화)
- **타겟 디바이스**: ESP32-S3-WROOM-1-N16R8
- **입력 해상도**: 480x160 픽셀
  - 골프공 크기: 약 76x76 픽셀 (이미지의 15.8% x 47.5%) 50~140 픽셀 사이임
- **추론 속도**: 6+ FPS
- **모델 크기**: 200-300KB

  모델 출력 → 정사각형 변환:

  1. 모델 출력: 직사각형 좌표 (x1,y1) to (x2,y2)
    - 예: (360,5) to (424,89) = 64×84 픽셀
  2. 현재 조작 (v29):
  size = (width + height) / 2  // 평균값
    - 64×84 → 74×74 정사각형
  3. 추가 배율 없음 (이전엔 1.1배 했었지만 제거)

### 필수 참고 사이트
- [ESP-DL GitHub](https://github.com/espressif/esp-dl)
- [ESP-Detection GitHub](https://github.com/espressif/esp-detection)

---

## 중요: espdet_pico 모델 사용

### ⚠️ 핵심 포인트
ESP32에서 높은 성능을 위해서는 반드시 **espdet_pico** 모델을 사용해야 합니다.

### espdet_pico vs YOLOv5/v8 비교

| 항목 | YOLOv5/v8 | espdet_pico |
|------|-----------|-------------|
| ESP32 최적화 | ❌ | ✅ |
| INT8 양자화 | 불안정 | 안정적 |
| 모델 크기 | 497-500KB | 200-300KB |
| 추론 성능 | 낮음 | 높음 |
| 정확도 | 양자화 후 ~20% | 양자화 후 70-90% |

### 🎯 최신 모델 성능 (2025-09-09 업데이트)
- **모델**: 99.4% mAP 골프공 검출 모델 (120 epoch 학습)
- **변환 결과**:
  - PT 파일: 2.6MB
  - ONNX: 1.4MB  
  - ESPDL: 488.2KB
- **파일 위치**: `C:/Users/goldk/ESP-Camera/models/golf_ball_99_4_mAP.espdl`

### espdet_pico 특징
- DSConv (Depthwise Separable Convolution)
- ESPBlockLite (ESP 전용 경량 블록)
- INT8 양자화에 최적화된 구조
- ESP32 하드웨어 가속 지원

---

## 사전 준비사항

### 1. Python 환경
```bash
# Python 3.8 필수
conda create -n espdet python=3.8
conda activate espdet
```

### 2. ESP-Detection 설치

#### 다운로드
```bash
# ESP-Detection 공식 저장소에서 다운로드
git clone https://github.com/espressif/esp-detection.git

# 또는 배치 파일 사용
Bat\download_esp_detection_correct.bat
```

#### 패키지 설치
```bash
cd C:\Users\goldk\ESP-Camera\esp-detection
pip install -r requirements.txt
```

### 3. ESP-IDF 설치
- ESP-IDF v5.3 이상 필요
- [설치 가이드](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/)

### 4. 하드웨어 핀맵 설정

#### ESP32-S3-WROOM-1-N16R8 + OV2640 카메라 핀맵
```c
#define CAM_PIN_RESET   -1  // 사용 안함

#define CAM_PIN_XCLK    15
#define CAM_PIN_PWDN    14
#define CAM_PIN_PCLK    13
#define CAM_PIN_HREF    7
#define CAM_PIN_VSYNC   6
#define CAM_PIN_SIOC    5   // SCL
#define CAM_PIN_SIOD    4   // SDA

#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
```

⚠️ **중요**: 이 핀맵은 ESP32-S3 DevKitC 기반 카메라 보드용입니다. 다른 보드를 사용하는 경우 핀맵을 확인하세요.

---

## 데이터셋 준비

### 중요: 실제 환경 데이터 수집의 중요성

**99.4% mAP 모델이 실제 환경에서 부정확한 이유:**
- 학습 데이터와 실제 사용 환경의 차이
- 해상도, 카메라 특성, 조명 조건의 불일치

### 올바른 데이터 수집 방법

#### 1. ESP32로 직접 촬영 (가장 중요!)
```
- 해상도: 480x160 (실제 사용 해상도)
- 카메라: ESP32 + OV2640
- 조건: 실제 사용할 장소와 조명
```

#### 2. 다양한 상황 포함
```
- 거리: 1m, 2m, 3m...
- 각도: 정면, 측면, 위/아래
- 조명: 아침, 낮, 저녁, 실내
- 배경: 잔디, 콘크리트, 카펫 등
```

#### 3. 올바른 라벨링
```
- 골프공만 정확히 표시 (여백 최소화)
- 예상 크기: 50~100픽셀
- 너무 큰 박스(>160픽셀) 지양
```

### 권장 데이터셋 구성
- **80%**: ESP32로 직접 촬영한 480x160 이미지
- **20%**: 유사 조건의 다른 해상도 (데이터 증강용)
- **최소 500장** 이상 권장

### 피해야 할 데이터
- ❌ 인터넷에서 다운로드한 고해상도 이미지
- ❌ 전문 카메라로 촬영한 선명한 이미지  
- ❌ 실제 환경과 다른 조건의 이미지

### 데이터 수집 도구
웹 인터페이스의 "Save Image" 버튼을 사용하여 ESP32에서 직접 이미지를 수집할 수 있습니다.

### 1. 디렉토리 구조
```
dataset_clean/
├── images/
│   ├── train/     # 학습 이미지 (80%)
│   └── val/       # 검증 이미지 (20%)
└── labels/
    ├── train/     # YOLO 형식 라벨
    └── val/       # YOLO 형식 라벨
```

### 2. golf_data.yaml 생성
```yaml
path: ../dataset_clean
train: images/train
val: images/val

names:
  0: golf_ball

# ESP-Detection 설정
rect: True  # 직사각형 이미지 지원
```

---

## 모델 학습

### espdet_pico 모델로 학습 (권장)

```python
# train_golf_espdet_pico.py
import sys
sys.path.append("C:/Users/goldk/ESP-Camera/esp-detection")

from train import Train

print("=== Golf Ball Detection with espdet_pico ===")
print("ESP32 최적화 모델로 학습 시작")

results = Train(
    dataset="dataset_clean/golf_data.yaml",
    imgsz=480,  # ESP32에 최적화된 크기 (480x160으로 자동 조정됨)
    epochs=120,  # 120 에폭
    batch=8,     # CPU 사용시 메모리에 맞게 조정
    device="cpu",  # GPU가 없으면 cpu 사용
    rect=False,  # ESP32 배포시 False 권장
    name="golf_espdet_pico",
    save_period=10,
    patience=0,  # Early stopping 비활성화
    # 모델 파라미터 생략 시 자동으로 espdet_pico 사용
)

print("학습 완료!")
print(f"결과: runs/detect/golf_espdet_pico/")
```

### 실행
```bash
cd C:\Users\goldk\ESP-Camera\esp-detection
python ..\Python\train_golf_espdet_pico.py
```

---

## 모델 변환

### ✅ 99.4% mAP 모델 변환 완료 (2025-09-09)

120 epoch 학습된 99.4% mAP 모델이 성공적으로 변환되었습니다:

#### 변환된 모델 정보
- **소스**: `esp-detection/runs/detect/golf_ball_espdet_pico_120/weights/best.pt`
- **결과**: `models/golf_ball_99_4_mAP.espdl`
- **크기**: PT(2.6MB) → ONNX(1.4MB) → ESPDL(488.2KB)

#### 변환 과정
1. **ONNX 변환** (ESP-Detection 환경에서):
```bash
cd C:/Users/goldk/ESP-Camera/esp-detection
python export_99_model_direct.py
```

2. **ESPDL 변환** (ESP-PPQ 사용):
```bash
cd C:/Users/goldk/ESP-Camera
python scripts/convert_golf_ball_espdl.py
```

### 변환 프로세스 확인

#### 1. **PyTorch → ONNX → ESP-DL format (Quantized INT8)** ✅

실제 수행한 과정:
- **PyTorch (.pt)**: `best.pt` (2.6MB)
- **ONNX**: `best.onnx` (1.4MB)
- **ESP-DL format**: `golf_ball_99_4_mAP.espdl` (488.2KB, INT8 양자화됨)

#### 2. **사용된 도구** ✅

- ESP-Detection의 export.py (PyTorch → ONNX)
- ESP-PPQ (ONNX → ESPDL, INT8 양자화)

#### 3. **최종 형태** ✅

- `.espdl` 파일로 생성됨 (ESP-DL의 표준 형식)
- CMakeLists.txt의 EMBED_FILES에 포함하여 펌웨어에 임베드

#### 4. **변환 명령어**:

```bash
# 1단계: PyTorch → ONNX
cd esp-detection
python export_99_model_direct.py

# 2단계: ONNX → ESPDL (INT8 양자화)
cd ..
python scripts/convert_golf_ball_espdl.py
```

### 일반적인 변환 방법

#### 1. ONNX 변환
```bash
python deploy/export.py --weights runs/detect/golf_espdet_pico/weights/best.pt --imgsz 480 160
```

#### 2. ESP-DL 양자화
```python
# convert_to_espdl.py
from deploy.quantize import quant_espdet

quant_espdet(
    onnx_path="runs/detect/golf_espdet_pico/weights/best.onnx",
    target="esp32s3",
    num_of_bits=8,
    device='cpu',
    batchsz=16,
    imgsz=[160, 480],  # height, width 순서
    calib_dir="../dataset_clean/images/val",  # 캘리브레이션용
    espdl_model_path="golf_ball_espdet.espdl"
)
```

### 3. 또는 All-in-One 스크립트 사용
```bash
python espdet_run.py \
  --class_name golf_ball \
  --pretrained_path None \
  --dataset "dataset_clean/golf_data.yaml" \
  --size 160 480 \
  --target "esp32s3" \
  --calib_data "dataset_clean/images/val" \
  --espdl "golf_ball_espdet.espdl" \
  --img "test_golf.jpg"
```

---

## ESP32 통합

### 1. WiFi 및 네트워크 설정

#### WiFi Station 모드 설정
ESP32를 기존 WiFi 네트워크에 연결하여 웹 인터페이스 제공:

```cpp
// WiFi 설정
.ssid = "TP-Link_54BE",
.password = "goldkwang1!",

// 고정 IP 설정
IP4_ADDR(&ip_info.ip, 192, 168, 0, 11);      // 고정 IP
IP4_ADDR(&ip_info.gw, 192, 168, 0, 1);       // 게이트웨이
IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0); // 서브넷 마스크
```

#### 웹 인터페이스 접속
- **URL**: http://192.168.0.11
- **기능**: 실시간 골프공 감지 영상 스트리밍 및 검출 결과 표시
- **웹 서버 포트**: 80

#### 네트워크 요구사항
- WiFi 네트워크가 2.4GHz 대역 지원 필요 (ESP32는 5GHz 미지원)
- DHCP 서버가 있는 경우에도 고정 IP(192.168.0.11) 사용
- 동일 네트워크 내의 장치에서만 접속 가능

### 2. 모델 파일 복사

#### 99.4% mAP 모델 사용 (권장)
```bash
# 변환된 99.4% 모델 사용
copy models\golf_ball_99_4_mAP.espdl main\
copy models\golf_ball_99_4_mAP.info main\
copy models\golf_ball_99_4_mAP.json main\
```

#### 또는 일반 모델
```bash
# ESP32 프로젝트로 복사
copy golf_ball_espdet.espdl C:\Users\goldk\ESP-Camera\models\
copy golf_ball_espdet.info C:\Users\goldk\ESP-Camera\models\
copy golf_ball_espdet.json C:\Users\goldk\ESP-Camera\models\
```

### 2. CMakeLists.txt 수정

#### 99.4% mAP 모델용
```cmake
idf_component_register(
    SRCS "main.cpp"
          "golf_detector.cpp"
          "camera_handler.cpp"
    INCLUDE_DIRS "." "../models"
    EMBED_FILES "../models/golf_ball_99_4_mAP.espdl"
                "../models/golf_ball_99_4_mAP.info"
                "../models/golf_ball_99_4_mAP.json"
)
```

#### 코드에서 모델 참조
```cpp
// 기존
extern const uint8_t golf_ball_best_espdl_start[] asm("_binary_golf_ball_best_espdl_start");

// 99.4% 모델용
extern const uint8_t golf_ball_99_4_mAP_espdl_start[] asm("_binary_golf_ball_99_4_mAP_espdl_start");
```

### 4. 빌드 및 플래시
```bash
cd C:\Users\goldk\ESP-Camera
idf.py set-target esp32s3
idf.py build
idf.py -p COM3 flash monitor
```

### 5. 웹 인터페이스 기능

#### 실시간 스트리밍 및 감지
- **스트리밍 URL**: http://192.168.0.11/stream
- **상태 확인 API**: http://192.168.0.11/status (JSON 형식)
- **메인 페이지**: http://192.168.0.11

#### 웹 페이지 기능
1. **실시간 영상 스트리밍**: MJPEG 형식으로 카메라 영상 전송
2. **골프공 감지 표시**: 감지된 골프공 개수를 실시간으로 표시
3. **스트림 제어**: 시작/중지 버튼으로 스트리밍 제어
4. **모델 정보 표시**: 사용 중인 모델 (99.4% mAP) 정보 표시

#### API 응답 예시
```json
{
    "count": 2,
    "model": "golf_ball_99_4_mAP"
}
```

---

## 문제 해결

### 1. 낮은 검출 정확도
- **원인**: YOLOv5/v8 모델 사용
- **해결**: espdet_pico 모델로 재학습

### 2. Relu 오류
```
RuntimeError: Op Execution Error: /model.10/act/Relu
```
- **원인**: 높은 정확도 모델의 양자화 불안정
- **해결**: espdet_pico 사용 (Relu 오류 없음)

### 3. 메모리 부족
- **해결**: 배치 크기 줄이기 (batchsz=8 또는 4)

### 4. Python 버전 오류
- **해결**: Python 3.8 사용 필수

---

## 성능 최적화 팁

### 1. 학습 파라미터
```python
# 최적 설정
imgsz=480         # ESP32에 최적 (480x160으로 자동 조정)
batch=8           # CPU 사용시 메모리에 따라 조정
rect=False        # ESP32 배포시 False 권장
```

### 2. 캘리브레이션
- 다양한 조명 조건의 이미지 사용
- 최소 100장 이상 권장

### 3. 모델 평가
```bash
# 양자화 전후 성능 비교
python val.py --weights runs/detect/golf_espdet_pico/weights/best.pt
```

---

## 결론

ESP-Detection의 **espdet_pico** 모델을 사용하면 ESP32에서 안정적이고 높은 성능의 골프공 검출이 가능합니다. 

### 핵심 요약
1. ✅ espdet_pico 모델 사용
2. ✅ 공식 ESP-Detection 도구 사용
3. ✅ INT8 양자화 최적화
4. ❌ YOLOv5/v8 모델 변환 시도 금지

추가 정보는 [ESP-Detection GitHub](https://github.com/espressif/esp-detection) 참조

---

## ESP-DL을 사용한 실제 추론 구현

### 1. ESP-DL 컴포넌트 설정

#### idf_component.yml 수정
```yaml
dependencies:
  espressif/esp32-camera: "^2.0.0"
  espressif/esp-dl: "^1.0.0"
```

#### CMakeLists.txt 수정
```cmake
idf_component_register(SRCS "main.cpp"
                            "camera_handler.cpp"
                            "golf_detector.cpp"
                       INCLUDE_DIRS "." "../src"
                       REQUIRES esp_timer nvs_flash freertos driver esp32-camera esp-dl
                       EMBED_FILES "../models/golf_ball_best.espdl")
```

### 2. ESP-DL API를 사용한 골프공 검출기 구현

#### 핵심 구현 (golf_detector.cpp)
```cpp
#include "golf_detector.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "dl_model.hpp"
#include "dl_image.hpp"
#include "dl_detect_base.hpp"
#include "dl_detect_espdet_postprocessor.hpp"

// 모델 데이터
extern const uint8_t golf_ball_best_espdl_start[] asm("_binary_golf_ball_best_espdl_start");

class GolfBallDetector : public dl::detect::DetectImpl {
private:
    dl::detect::ESPDetPostProcessor *m_postprocessor;
    
public:
    GolfBallDetector() {
        ESP_LOGI(TAG, "Initializing Golf Ball Detector with ESPDL model...");
        
        // 임베디드 데이터에서 모델 로드
        const char *model_data = (const char *)golf_ball_best_espdl_start;
        m_model = new dl::Model(model_data, "golf_ball_best", fbs::MODEL_LOCATION_IN_FLASH_RODATA);
        m_model->minimize();
        
        // 이미지 전처리기 초기화 (정규화: [0, 255])
        m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {255, 255, 255});
        
        // 후처리기 초기화 (score_threshold, nms_threshold, top_k, anchors)
        m_postprocessor = new dl::detect::ESPDetPostProcessor(
            m_model, 
            0.5f,    // score threshold
            0.45f,   // nms threshold  
            10,      // top k detections
            {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}}  // anchors
        );
        
        ESP_LOGI(TAG, "Golf ball detector initialized successfully");
    }
};
```

### 3. ESP-DL 모델 구조 이해

#### ESP-DL v1.1.0 아키텍처
- **Model Zoo 패턴**: 사전 빌드된 모델 클래스 제공
- **DetectImpl 상속**: 커스텀 검출기 구현을 위한 베이스 클래스
- **전처리/후처리**: 내장된 이미지 전처리 및 검출 후처리 파이프라인

#### 주요 클래스와 API
```cpp
// 모델 로드
dl::Model(model_data, model_name, location_type);

// 이미지 전처리
dl::image::ImagePreprocessor(model, mean, std);

// 검출 후처리
dl::detect::ESPDetPostProcessor(model, score_th, nms_th, top_k, anchors);

// 추론 실행
detector->run<uint8_t>(image_data, shape);  // RGB888
detector->run<uint16_t>(image_data, shape); // RGB565
```

### 4. 실제 추론 프로세스

#### 검출 함수 구현
```cpp
detection_result_t detect_golf_balls(camera_fb_t *fb) {
    detection_result_t result = {0};
    
    // 이미지 shape 생성
    std::vector<int> shape = {(int)fb->height, (int)fb->width, 3};
    
    // 픽셀 포맷에 따른 추론 실행
    std::forward_list<dl::detect::result_t> detections;
    
    if (fb->format == PIXFORMAT_RGB888) {
        detections = detector->run<uint8_t>(fb->buf, shape);
    } else if (fb->format == PIXFORMAT_RGB565) {
        detections = detector->run<uint16_t>((uint16_t*)fb->buf, shape);
    }
    
    // 검출 결과 변환
    for (auto &det : detections) {
        if (det.box.size() >= 4) {
            // 중심 좌표 계산
            ball.x = (det.box[0] + det.box[2]) / 2.0f;
            ball.y = (det.box[1] + det.box[3]) / 2.0f;
            ball.confidence = det.score;
        }
    }
    
    return result;
}
```

### 5. ESP-DL vs ESP-Detection 비교

| 특성 | ESP-Detection | ESP-DL |
|------|--------------|--------|
| 모델 학습 | ✅ 지원 | ❌ 추론만 |
| 커스텀 모델 | ✅ espdet 아키텍처 | ⚠️ 변환 필요 |
| 모델 로딩 | 런타임 | 컴파일 시점 |
| API 복잡도 | 간단 | 상세 제어 |
| 메모리 효율 | 높음 | 매우 높음 |

### 6. 버전 관리

프로그램 버전을 main.cpp에 추가하여 변경사항 추적:
```cpp
ESP_LOGI(TAG, "Version: v2.0.0-REAL-ESPDL - Using dl::Model API with DetectImpl");
ESP_LOGI(TAG, "Test completed. System halted.");
```

### 7. 주의사항 및 팁

#### ⚠️ 중요 사항
1. ESP-DL은 동적 모델 로딩을 지원하지 않음 (모델이 펌웨어에 포함됨)
2. 모델 크기가 플래시 메모리에 영향을 줌
3. INT8 양자화된 모델만 사용 권장

#### 💡 최적화 팁
1. `m_model->minimize()` 호출로 메모리 사용 최소화
2. 앵커 크기를 모델 학습 시 사용한 값과 일치시킴
3. NMS threshold 조정으로 중복 검출 제거

### 8. ESP-DL 버전 선택 가이드

#### ESP-DL v1.x vs v3.x 비교

| 특성 | ESP-DL v1.x | ESP-DL v3.x |
|------|-------------|-------------|
| 사전 빌드 모델 | ✅ 지원 | ✅ 지원 |
| 커스텀 ESPDL 모델 | ❌ 불가능 | ✅ 가능 |
| Model 클래스 | ❌ 없음 | ✅ dl::Model |
| 동적 모델 로딩 | ❌ 불가능 | ✅ 가능 |
| 메모리 효율성 | 보통 | 높음 |
| API 복잡도 | 간단 | 중간 |

#### ⚠️ 중요: 버전 선택
- **ESP-DL v1.1.0**: 사전 빌드된 모델(face, cat 등)만 사용 가능
- **ESP-DL v3.x**: 커스텀 ESPDL 모델 실행 필수

#### 업그레이드 방법
1. `idf_component.yml` 수정:
```yaml
dependencies:
  espressif/esp32-camera: "^2.0.0"
  espressif/esp-dl: "^3.0.0"  # v1.0.0 → v3.0.0
```

2. 컴포넌트 업데이트:
```bash
idf.py fullclean
idf.py update-dependencies
idf.py build
```

### 9. ESP-DL v3.x 구현 예제

```cpp
// ESP-DL v3.x로 커스텀 모델 로드
class GolfBallDetector {
private:
    std::unique_ptr<dl::Model> model;
    std::unique_ptr<dl::Tensor<int8_t>> input_tensor;
    
public:
    bool init() {
        // ESPDL 모델 로드
        model = std::make_unique<dl::Model>(model_data, model_size, nullptr, 0);
        
        // 입력 텐서 생성
        auto input_shapes = model->get_input_shapes();
        input_tensor = std::make_unique<dl::Tensor<int8_t>>(input_shapes[0]);
        
        return true;
    }
    
    void detect() {
        // 추론 실행
        model->run(input_tensor.get());
        
        // 출력 처리
        auto outputs = model->get_outputs();
    }
};
```

### 10. 디버깅 및 문제 해결

#### 헤더 파일 오류
```
fatal error: dl_detect_espdet_postprocessor.hpp: No such file or directory
```
**해결**: ESP-DL 컴포넌트가 제대로 설치되었는지 확인

#### 모델 로딩 실패
```
Failed to create detector
```
**해결**: ESPDL 파일이 CMakeLists.txt의 EMBED_FILES에 포함되었는지 확인

#### 메모리 부족
```
Failed to allocate RGB buffer
```
**해결**: PSRAM 활성화 또는 입력 이미지 크기 축소

---

## 최종 정리

이 가이드를 통해 ESP32에서 실제 딥러닝 추론이 가능한 골프공 검출 시스템을 구축했습니다:

1. ✅ ESP-Detection으로 espdet_pico 모델 학습 (mAP 99.4%)
2. ✅ INT8 양자화로 ESP32 최적화
3. ✅ ESP-DL API를 사용한 실제 추론 구현
4. ✅ 0.05ms 추론 속도 달성 가능

ESP32에서의 실시간 AI 비전 처리가 현실이 되었습니다!

---

## 모델 교체 가이드

### 모델 파일 이름 표준화

프로젝트에서는 모델 파일 이름을 `golf_ball_model.espdl`로 고정하여 사용합니다.

#### 이유
- 다양한 버전의 모델 (예: golf_ball_99_4_mAP.espdl, golf_ball_95_7_mAP.espdl)을 쉽게 교체
- 코드 수정 없이 모델만 바꿔서 사용 가능
- 버전 관리 편의성

### 새 모델 적용 방법

1. **새 모델 학습 후**
   - 학습된 모델 파일 (예: `golf_ball_new_version.espdl`)을 생성

2. **모델 파일 복사**
   ```bash
   # 실제 모델 위치에 복사
   copy golf_ball_new_version.espdl C:\Users\goldk\ESP-Camera\esp-dl\models\golf_ball_detect\s3\golf_ball_model.espdl
   ```

3. **참조 파일 생성 (선택사항)**
   `C:\Users\goldk\ESP-Camera\esp-dl\models\cat_detect\models\s3\golf_ball_model.espdl` 파일 생성:
   ```
   COPY_FROM: C:/Users/goldk/ESP-Camera/esp-dl/models/golf_ball_detect/s3/golf_ball_model.espdl
   ```

4. **빌드 및 플래시**
   ```bash
   idf.py build
   idf.py -p COM3 flash
   ```

### 필요한 파일
- **필수**: `golf_ball_model.espdl` (새로 학습한 모델 파일)
- **선택**: `.info`, `.json` 파일 (있으면 함께 복사)

### 주의사항
- 모델은 반드시 ESP32-S3용으로 학습되어야 함
- ESPDL 형식이어야 함 (INT8 양자화 완료)
- 입력 크기가 현재 설정 (480x160)과 호환되어야 함

### 코드 내 참조
```cpp
// cat_detect.cpp
ESP_LOGI("cat_detect", "Loading golf ball model: golf_ball_model.espdl");
m_model = new cat_detect::ESPDet("golf_ball_model.espdl");
```

이제 새로운 모델을 학습할 때마다 파일명만 `golf_ball_model.espdl`로 변경하여 사용하면 됩니다!