@echo off
echo Creating model directory structure...
mkdir models\s3 2>nul

echo Copying golf ball model...
copy "C:\Users\goldk\ESP-Camera\esp-dl\models\golf_ball_detect\s3\golf_ball_99_4_mAP.espdl" "models\s3\"

echo Done!