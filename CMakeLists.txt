cmake_minimum_required(VERSION 3.15)
project(ShaderCompiler VERSION 1.0)

# 设置编译器选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# VS2022的工具集设置
set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE INTERNAL "" FORCE)
set(CMAKE_GENERATOR_PLATFORM x64 CACHE INTERNAL "" FORCE)

# 设置Qt路径 - 使用Qt 5.12.5
set(Qt5_DIR "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64/lib/cmake/Qt5")
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 添加Qt的include路径
include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64/include"
    "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64/include/QtWidgets"
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加资源文件
set(RESOURCE_FILES
    resources.qrc
)

qt5_add_resources(QRC_SOURCES ${RESOURCE_FILES})

# 源文件
add_executable(ShaderCompiler
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/fxcCompiler.cpp
    src/fxcCompiler.h
    src/compilerConfig.h
    src/compilerConfig.cpp
    src/languageConfig.h
    src/languageConfig.cpp
    src/compilerSettingUI.h
    src/compilerSettingUI.cpp
    ${QRC_SOURCES}
)

# 链接Qt库
target_link_libraries(ShaderCompiler PRIVATE
    Qt5::Widgets
)

# 添加Qt的运行时DLL路径
set(QT_INSTALL_PATH "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64")
set(QT_BIN_PATH "${QT_INSTALL_PATH}/bin")
set(QT_PLUGIN_PATH "${QT_INSTALL_PATH}/plugins")

# 复制Qt运行时DLL到输出目录
add_custom_command(TARGET ShaderCompiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_BIN_PATH}/Qt5Widgets.dll
        ${QT_BIN_PATH}/Qt5Core.dll
        ${QT_BIN_PATH}/Qt5Gui.dll
        $<TARGET_FILE_DIR:ShaderCompiler>
)

# 安装配置
install(TARGETS ShaderCompiler
    RUNTIME DESTINATION bin
)

# 在项目设置之后添加
if(MSVC)
    add_compile_options(/utf-8)
endif() 