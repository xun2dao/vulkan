cmake_minimum_required(VERSION 3.12.0)
project(Vulkan VERSION  1.0) # 会生成两个变量Vulkan_VERSION_MAJOR Vulkan_VERSION_MINOR

# 指定变量
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(LIBS glfw vulkan dl pthread X11 Xxf86vm Xrandr Xi)
set(CMAKE_BUILD_TYPE Debug)

add_executable(app main.cpp)

target_link_libraries(app ${LIBS})
target_compile_definitions(app PUBLIC $<$<CONFIG:Debug>: DEBUG>)
