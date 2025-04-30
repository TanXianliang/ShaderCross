@echo off
:: 定义源文件夹路径
set "source_folder=..\build\Release"

:: 定义目标文件夹路径（包含新名称）
set "destination_folder=shader-cross"

:: 检查源文件夹是否存在
if not exist "%source_folder%" (
    echo 源文件夹不存在: %source_folder%
    exit /b 1
)

:: 复制文件夹到目标路径
robocopy "%source_folder%" "%destination_folder%" /E /COPYALL /R:3 /W:5

:: 检查复制是否成功
if %errorlevel% leq 7 (
    echo 文件夹已成功复制并重命名为: %destination_folder%
) else (
    echo 文件夹复制失败
    exit /b 1
)

c:\Qt\Qt5.12.5\5.12.5\msvc2017_64\bin\windeployqt.exe shader-cross\ShaderCross.exe --release