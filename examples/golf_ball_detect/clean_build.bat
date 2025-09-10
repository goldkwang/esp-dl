@echo off
if exist build (
    echo Deleting build directory...
    rmdir /s /q build
    echo Build directory deleted.
) else (
    echo Build directory does not exist.
)