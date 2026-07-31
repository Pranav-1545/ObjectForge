
cmake_minimum_required(VERSION 3.22)

# Qt6 Configuration
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
qt_standard_project_setup()

set(APP_SOURCES
    src/main.cpp
)

qt_add_executable(ObjectForgeApp ${APP_SOURCES})

target_link_libraries(ObjectForgeApp PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)
add_custom_command(TARGET ObjectForgeApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ObjectForgeApp>/platforms
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/Qt6/plugins/platforms/qwindows.dll"
        "$<TARGET_FILE_DIR:ObjectForgeApp>/platforms/qwindows.dll"
)