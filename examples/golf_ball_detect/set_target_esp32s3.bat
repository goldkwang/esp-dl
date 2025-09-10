@echo off
echo Cleaning build directory...
if exist build (
    rmdir /s /q build
    echo Build directory removed.
) else (
    echo Build directory does not exist.
)

echo.
echo Setting target to ESP32-S3...
call idf.py set-target esp32s3

echo.
echo Target set to ESP32-S3. You can now run 'idf.py build'