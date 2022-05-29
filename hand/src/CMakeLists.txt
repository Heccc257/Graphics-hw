# link_directories(
#         "C:/Users/27623/Desktop/Graphics/Hand-cmake/hand-graphics-homework-main/src"
# )

add_executable(Hand
        gl_env.h
        main.cpp
        skeletal_mesh.h
        texture_image.h
        # SOIL.h
        )


target_link_libraries(Hand PRIVATE assimp::assimp glew_s glm stb glfw imgui)
target_include_directories(Hand PRIVATE
        ../third_party/glew/include
        ../third_party/imgui/
        # ../third_party/soil/
        ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(Hand PRIVATE cxx_std_11)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)