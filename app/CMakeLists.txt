# Add application executable
add_executable(ObjectForgeApp
    src/main.cpp
    src/GrpcClient.h
    src/GrpcClient.cpp
    src/CameraWorker.h
    src/CameraWorker.cpp
    src/ClickableLabel.h
    src/ClickableLabel.cpp
)

# Include directories for app target
target_include_directories(ObjectForgeApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt, OpenCV, gRPC, and generated proto library
target_link_libraries(ObjectForgeApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${OpenCV_LIBS}
    pipeline_proto
)