find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL)
find_package(OpenCV REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(ObjectForgeApp
    src/main.cpp
    src/GrpcClient.cpp
    src/GrpcClient.h
    src/CameraWorker.cpp
    src/CameraWorker.h
)

target_include_directories(ObjectForgeApp PRIVATE
    src
)

target_link_libraries(ObjectForgeApp PRIVATE
    Qt6::Widgets
    Qt6::OpenGL
    ${OpenCV_LIBS}
    pipeline_proto
)

# Copy qwindows.dll post-build
add_custom_command(TARGET ObjectForgeApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ObjectForgeApp>/platforms
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms/qwindows.dll"
        "$<TARGET_FILE_DIR:ObjectForgeApp>/platforms/qwindows.dll"
)