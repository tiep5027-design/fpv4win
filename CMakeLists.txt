
cmake_minimum_required(VERSION 3.20)

project(fpv4win LANGUAGES C CXX)

# 1. Language standard
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

# Exclude third-party example main.cpp
list(FILTER SRC_LIST EXCLUDE
    REGEX ".*/3rd/rtl8812au-monitor-pcap/src/main\\.cpp$"
)

# Headless receiver:
# Remove original video player and encoder modules
list(FILTER SRC_LIST EXCLUDE
    REGEX ".*/src/player/.*"
)

# 5. Fix missing chrono header for RTL8812AU
if(MSVC)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src/HalModule.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src/RadioManagementModule.cpp"
        PROPERTIES COMPILE_OPTIONS "/FIchrono"
    )
endif()

# 6. Create executable
# qml.qrc is not needed for headless receiver
add_executable(${PROJECT_NAME} ${SRC_LIST})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/3rd/rtl8812au-monitor-pcap/hal"
    ${FFMPEG_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Quick
    Qt5::Multimedia
    ${FFMPEG_LIBRARIES}
    ${SDL2_LIBRARIES}
    PkgConfig::LIBUSB
    unofficial-sodium::sodium
)

# 7. Debug configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
)

# Runtime DLLs will be packaged separately
# after successful compilation.
