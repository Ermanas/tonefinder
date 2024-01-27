

add_executable(microphone_monitor microphone_monitor.cpp)
target_link_libraries(microphone_monitor PRIVATE PortAudio)

add_executable(microphone_monitor_class microphone_monitor_class.cpp)
target_link_libraries(microphone_monitor_class PRIVATE PortAudio)

add_executable(frequency_finder frequency_finder.cpp)
target_include_directories(frequency_finder PRIVATE ${PROJECT_SOURCE_DIR}/external/fftw-3.3.10/api)
target_link_libraries(frequency_finder PRIVATE PortAudio fftw3)