@echo off
echo Creating golf_ball_detect from cat_detect...

REM Copy entire cat_detect folder to golf_ball_detect
xcopy /E /I /Y cat_detect golf_ball_detect

echo Done! golf_ball_detect project created.
echo.
echo Next steps:
echo 1. Run this batch file
echo 2. Copy golf ball images to golf_ball_detect\main\
echo 3. Copy golf ball model to models\golf_ball_detect\