cmake_minimum_required(VERSION 3.15)
project(ShaderCross VERSION 1.0)

# 设置编译器选项
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# VS2022的工具集设置
set(CMAKE_GENERATOR "Visual Studio 17 2022" CACHE INTERNAL "" FORCE)
set(CMAKE_GENERATOR_PLATFORM x64 CACHE INTERNAL "" FORCE)

set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64")

# 设置Qt路径 - 使用Qt 5.12.5
# 添加Qt的运行时DLL路径
set(QT_INSTALL_PATH "C:/Qt/Qt5.12.5/5.12.5/msvc2017_64")
set(Qt5_DIR "${QT_INSTALL_PATH}/lib/cmake/Qt5")
set(QT_BIN_PATH "${QT_INSTALL_PATH}/bin")
set(QT_PLUGIN_PATH "${QT_INSTALL_PATH}/plugins")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 添加Qt的include路径
include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    "${QT_INSTALL_PATH}/include"
    "${QT_INSTALL_PATH}/include/QtWidgets"
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/fxc
)

# 添加资源文件
set(RESOURCE_FILES
    resources.qrc
)

qt5_add_resources(QRC_SOURCES ${RESOURCE_FILES})

# 设置 Windows 子系统
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# 源文件 - 所有配置都使用 WIN32
add_executable(ShaderCross WIN32   
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/fxcCompiler.cpp
    src/fxcCompiler.h
    src/dxcCompiler.cpp
    src/dxcCompiler.h
    src/glslangCompiler.cpp
    src/glslangCompiler.h
    src/glslangkgverCompiler.cpp
    src/glslangkgverCompiler.h
    src/glslkgverCodePrebuilder.cpp
    src/glslkgverCodePrebuilder.h
    src/compilerConfig.h
    src/compilerConfig.cpp
    src/languageConfig.h
    src/languageConfig.cpp
    src/compilerSettingUI.h
    src/compilerSettingUI.cpp
    src/shaderCodeTextEdit.h
    src/shaderCodeTextEdit.cpp
    src/documentWindow.h
    src/documentWindow.cpp
    src/spirvUtils.h
    src/spirvUtils.cpp
    src/SPIRV-Reflect/spirv_reflect.h
    src/SPIRV-Reflect/spirv_reflect.c
    ${QRC_SOURCES}
)

# 设置可执行文件的工作目录为其所在目录
set_target_properties(ShaderCross PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShaderCross>"
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:ShaderCross>"
)

# 链接Qt库
target_link_libraries(ShaderCross PRIVATE
    Qt5::Widgets
)

# 复制Qt运行时DLL到输出目录
add_custom_command(TARGET ShaderCross POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${QT_BIN_PATH}/Qt5Widgets.dll
        ${QT_BIN_PATH}/Qt5Core.dll
        ${QT_BIN_PATH}/Qt5Gui.dll
        $<TARGET_FILE_DIR:ShaderCross>
)

# 复制 fxc 依赖工具到输出目录
add_custom_command(TARGET ShaderCross POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/external/fxc/fxc.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/fxc/d3dcompiler_47.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/bin/x64/dxc.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/bin/x64/dxcompiler.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/bin/x64/dxil.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glslang/glslang.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glslang/glslangValidator.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-cross/spirv-cross.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv-cross/spirv-dis.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-as.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-cfg.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-dis.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-link.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-lint.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-objdump.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-opt.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-reduce.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-reflect.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-reflect-pp.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-remap.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/SPIRV-Tools-shared.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/SPIRV-Tools-sharedd.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spirv/spirv-val.exe
        $<TARGET_FILE_DIR:ShaderCross>
)

# 复制依赖文件和目录
add_custom_command(TARGET ShaderCross POST_BUILD
    # 复制 glslkgver 整个目录
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/external/glslkgver"
        "$<TARGET_FILE_DIR:ShaderCross>/external/glslkgver"
)

# 安装配置
install(TARGETS ShaderCross
    RUNTIME DESTINATION bin
)

# 在项目设置之后添加
if(MSVC)
    add_compile_options(/utf-8)
endif() 