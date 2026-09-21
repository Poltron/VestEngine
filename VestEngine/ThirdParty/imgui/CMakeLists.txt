add_library(imgui 
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.cpp"
	"${CMAKE_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp")
target_include_directories(imgui PUBLIC "${CMAKE_SOURCE_DIR}/ThirdParty/imgui")

target_link_libraries(imgui PRIVATE glad)
target_link_libraries(imgui PRIVATE glfw)

set_target_properties(imgui PROPERTIES FOLDER "ThirdParty")