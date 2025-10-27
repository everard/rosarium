cmake_minimum_required(VERSION 3.31)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")
set(CMAKE_CXX_MODULE_STD ON)

project(platformer-game CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("-Wall" "-Wextra" "-Wdeprecated" "-Wno-missing-field-initializers" "-O2" "--pedantic")

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(main.opengl)

target_sources(main.opengl
    PUBLIC
    src/main.opengl.cc
)

target_sources(main.opengl
    PUBLIC
    FILE_SET modules TYPE CXX_MODULES
    FILES
        src/rose.graphics.opengl.kernel.ccm
        src/rose.random.ccm
        src/rose.span.ccm
)

target_link_libraries(
    main.opengl PRIVATE
    c++
    GLEW
    OpenGL
    SDL2::SDL2
    SDL2::SDL2main
)


add_executable(main.vulkan)

target_sources(main.vulkan
    PUBLIC
    src/main.vulkan.cc
)

target_sources(main.vulkan
    PUBLIC
    FILE_SET modules TYPE CXX_MODULES
    FILES
        src/rose.graphics.vulkan.device.ccm
        src/rose.graphics.vulkan.kernel.ccm
        src/rose.graphics.vulkan.memory.ccm
        src/rose.graphics.vulkan.swapchain.ccm
        src/rose.random.ccm
        src/rose.span.ccm
)

target_link_libraries(
    main.vulkan PRIVATE
    c++
    vulkan
    SDL2::SDL2
    SDL2::SDL2main
)
