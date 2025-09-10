@echo off
echo ===== Golf Ball Detection Project Setup =====
echo.

echo 1. Copying cat_detect to golf_ball_detect...
copy_to_golf_ball.bat

echo.
echo 2. Copying golf ball images...
cd golf_ball_detect
mkdir img 2>nul
copy ..\cat_detect\img\golf_ball_00200.jpg img\
copy ..\cat_detect\img\golf_ball_00208.jpg img\
copy img\golf_ball_00200.jpg main\
copy img\golf_ball_00208.jpg main\

echo.
echo 3. Updating project files...
copy /Y ..\golf_ball_detect_files\main\CMakeLists.txt main\
copy /Y ..\golf_ball_detect_files\main\app_main.cpp main\
copy /Y ..\golf_ball_detect_files\sdkconfig.defaults .

echo.
echo 4. Copying golf ball model...
cd ..\..\models\golf_ball_detect
copy_model.bat

echo.
echo ===== Setup Complete! =====
echo.
echo Next steps:
echo 1. cd C:\Users\goldk\ESP-Camera\esp-dl\examples\golf_ball_detect
echo 2. del sdkconfig (if exists)
echo 3. idf.py build
echo 4. idf.py -p COM3 flash monitor
echo.
pause