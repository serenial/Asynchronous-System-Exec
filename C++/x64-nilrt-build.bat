set SCRIPT_DIR=%~dp0.
set IMAGE_NAME=serenial.io-ase-nilrt_x64
set BUILD_DIR="%SCRIPT_DIR%\..\.build-nilrt"

docker build -t %IMAGE_NAME% "%SCRIPT_DIR%\docker-nilrt-build\." ^
 && docker run -v "%SCRIPT_DIR%":"//C++" -v "%SCRIPT_DIR%\..\LabVIEW":"//LabVIEW" -v "%BUILD_DIR%":"//build-archive" -it --rm %IMAGE_NAME%