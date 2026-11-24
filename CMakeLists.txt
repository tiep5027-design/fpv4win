
cmake_minimum_required(VERSION 3.20)

project(fpv4win LANGUAGES C CXX)

# 1. C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Qt code generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 3. Dependencies
find_package(PkgConfig REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(SDL2 REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Quick Multimedia)

pkg_check_modules(
    LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0
)

# 4. Collect source files
file(GLOB_RECURSE SRC_LIST CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

# Exclude third-party main.cpp
list(FILTER SRC_LIST EXCLUDE
    REGEX ".*/3rd/rtl8812au-monitor-pcap/src/main\\.cpp$"
)

# Headless: exclude video player modules
list(FILTER SRC_LIST EXCLUDE
    REGEX ".*/src/player/.*"
)

# 5. Fix RTL8812AU chrono header
if(MSVC)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src/HalModule.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src/RadioManagementModule.cpp"
        PROPERTIES COMPILE_OPTIONS "/FIchrono"
    )
endif()

# 6. Create headless executable
add_executable(${PROJECT_NAME} ${SRC_LIST})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/hal"
    ${FFMPEG_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Quick
    Qt5::Multimedia
    ${FFMPEG_LIBRARIES}
    ${SDL2_LIBRARIES}
    PkgConfig::LIBUSB
    unofficial-sodium::sodium
)

# 7. FIX BUILD #7 - Windows Winsock library
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# 8. Debug configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# Runtime DLLs are packaged separately
# after successful compilation.
