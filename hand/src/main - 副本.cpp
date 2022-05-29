// Hand Example
// Author: Yi Kangrui <yikangrui@pku.edu.cn>

// #define DIFFUSE_TEXTURE_MAPPING

#include "gl_env.h"

#include <cstdlib>
#include <cstdio>
#include <config.h>

#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif

#include <iostream>

#include "skeletal_mesh.h"

#include <glm/gtc/matrix_transform.hpp>

#include<cmath>
#include<map>
#include<fstream>
#include<imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SOIL.h"


namespace SkeletalAnimation {
    const char *vertex_shader_330 =
            "#version 330 core\n"
            "const int MAX_BONES = 100;\n"
            "uniform mat4 u_bone_transf[MAX_BONES];\n"
            "uniform mat4 u_mvp;\n"
            "layout(location = 0) in vec3 in_position;\n"
            "layout(location = 1) in vec2 in_texcoord;\n"
            "layout(location = 2) in vec3 in_normal;\n"
            "layout(location = 3) in ivec4 in_bone_index;\n"
            "layout(location = 4) in vec4 in_bone_weight;\n"
            "out vec2 pass_texcoord;\n"
            "void main() {\n"
            "    float adjust_factor = 0.0;\n"
            "    for (int i = 0; i < 4; i++) adjust_factor += in_bone_weight[i] * 0.25;\n"
            "    mat4 bone_transform = mat4(1.0);\n"
            "    if (adjust_factor > 1e-3) {\n"
            "        bone_transform -= bone_transform;\n"
            "        for (int i = 0; i < 4; i++)\n"
            "            bone_transform += u_bone_transf[in_bone_index[i]] * in_bone_weight[i] / adjust_factor;\n"
            "	 }\n"
            "    gl_Position = u_mvp * bone_transform * vec4(in_position, 1.0);\n"
            "    pass_texcoord = in_texcoord;\n"
            "}\n";

    const char *fragment_shader_330 =
            "#version 330 core\n"
            "uniform sampler2D u_diffuse;\n"
            "in vec2 pass_texcoord;\n"
            "out vec4 out_color;\n"
            "void main() {\n"
            #ifdef DIFFUSE_TEXTURE_MAPPING
            "    out_color = vec4(texture(u_diffuse, pass_texcoord).xyz, 1.0);\n"
            #else
            "    out_color = vec4(pass_texcoord, 0.0, 1.0);\n"
            #endif
            "}\n";
}

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

const int max_scr = 230;
int cur_scr = 200;

glm::mat4 scale = glm::mat4(1.0f);

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    // std::cerr<<"xoffset "<<xoffset<<" yoffset "<<yoffset<<" "<<cur_scr <<"\n";
    cur_scr += yoffset;
    cur_scr = std::max(120, cur_scr);
    cur_scr = std::min(max_scr, cur_scr);
    float ratio = cur_scr / 200.0;
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(ratio, ratio, ratio));
}

std::string thumb_phalange[] = {"thumb_proximal_phalange", "thumb_intermediate_phalange", "thumb_distal_phalange", "thumb_fingertip"};
std::string index_phalange[] = {"index_proximal_phalange", "index_intermediate_phalange", "index_distal_phalange", "index_fingertip"};
std::string middle_phalange[] = {"middle_proximal_phalange", "middle_intermediate_phalange", "middle_distal_phalange", "middle_fingertip"};
std::string ring_phalange[] = {"ring_proximal_phalange", "ring_intermediate_phalange", "ring_distal_phalange", "ring_fingertip"};
std::string pinky_phalange[] = {"pinky_proximal_phalange", "pinky_intermediate_phalange", "pinky_distal_phalange", "pinky_fingertip"};
void Gesture_default(SkeletalMesh::SkeletonModifier &modifier) {
    for(auto phalange : thumb_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), 0.0f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : index_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), 0.0f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    
    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), 0.0f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
        
    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), 0.0f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), 0.0f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
}

void Gesture_OK(SkeletalMesh::SkeletonModifier &modifier, float ratio = 1) {
    std::map<std::string, float> gesture;
    gesture.clear();

    gesture["index_proximal_phalange"] =(float)(M_PI * 0.3);
    gesture["index_intermediate_phalange"] = (float)(M_PI * 0.25);
    gesture["index_distal_phalange"] = (float)(M_PI * 0.33);

    gesture["thumb_proximal_phalange"] = (float)(M_PI * 0.03);
    gesture["thumb_intermediate_phalange"] = (float)(M_PI * 0.26);

    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), -(float)(0.06f * ratio * M_PI),
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), -(float)(0.06f * ratio * M_PI),
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), -(float)(0.06f * ratio * M_PI),
                                                            glm::fvec3(0.0, 0.0, 1.0));


    // std::cerr<<" ratio = "<<ratio<<"\n";
    for(auto it:gesture) {
        modifier[it.first] = glm::rotate(glm::identity<glm::mat4>(), ratio * it.second,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    }
}


void Gesture_pistol(SkeletalMesh::SkeletonModifier &modifier, float ratio = 1) {
    float angle = M_PI * 0.35;

    angle *= ratio;

    modifier["thumb_proximal_phalange"] = glm::rotate(glm::identity<glm::mat4>(), -(float)(M_PI * 0.1f * ratio),
                                                            glm::fvec3(0.0, 0.0, 1.0));


    modifier["thumb_distal_phalange"] = glm::rotate(glm::identity<glm::mat4>(), -(float)(M_PI * 0.14f * ratio),
                                                            glm::fvec3(0.0, 0.0, 1.0));
    modifier["index_intermediate_phalange"] = glm::rotate(glm::identity<glm::mat4>(), -(float)(M_PI * 0.1f * ratio),
                                                            glm::fvec3(0.0, 0.0, 1.0));
    
    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.2f,
                                                            glm::fvec3(0.0, 0.0, 1.0));

    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
}

void Gesture_L(SkeletalMesh::SkeletonModifier &modifier, float ratio = 1) {
    float angle = M_PI * 0.35;

    angle *= ratio;

    modifier["thumb_proximal_phalange"] = glm::rotate(glm::identity<glm::mat4>(), angle * 0.55f,
                                                            glm::fvec3(0.0, 0.0, 1.0));


    modifier["thumb_distal_phalange"] = glm::rotate(glm::identity<glm::mat4>(), -(float)(M_PI * 0.164f * ratio),
                                                            glm::fvec3(0.0, 0.0, 1.0));
    
    modifier["index_proximal_phalange"] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.2f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    modifier["index_intermediate_phalange"] = glm::rotate(glm::identity<glm::mat4>(), angle * 0.3f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    
    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.2f,
                                                            glm::fvec3(0.0, 0.0, 1.0));

    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
}

void Gesture_point(SkeletalMesh::SkeletonModifier &modifier, float ratio) {
    Gesture_default(modifier);
    float angle = ratio * M_PI * 0.35;

    
    // modifier["index_intermediate_phalange"] = glm::rotate(glm::identity<glm::mat4>(), -(float)(M_PI * 0.1f),
    //                                                         glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : thumb_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.2f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.2f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle * 1.15f,
                                                            glm::fvec3(0.0, 0.0, 1.0));
}

void scratch(SkeletalMesh::SkeletonModifier &modifier, float angle) {
    for(auto phalange : thumb_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : index_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : middle_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : ring_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle,
                                                            glm::fvec3(0.0, 0.0, 1.0));
    for(auto phalange : pinky_phalange) 
        modifier[phalange] = glm::rotate(glm::identity<glm::mat4>(), angle,
                                                            glm::fvec3(0.0, 0.0, 1.0));
}

// void Camera_move(GLFWwindow* window, glm::mat4 &trans) {
//     if(glfwGetKey(window, GLFW_KEY_RIGHT) | glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
//         trans = glm::translate(trans, glm::vec3(0.01f, 0.0f, 0.0f));
//     }
//     if(glfwGetKey(window, GLFW_KEY_LEFT) | glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
//         trans = glm::translate(trans, glm::vec3(-0.01f, 0.0f, 0.0f));
//     }
//     if(glfwGetKey(window, GLFW_KEY_UP) | glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
//         trans = glm::translate(trans, glm::vec3(0.0f, 0.01f, 0.0f));
//     }
//     if(glfwGetKey(window, GLFW_KEY_DOWN) | glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
//         trans = glm::translate(trans, glm::vec3(0.0f, -0.01f, 0.0f));
//     }
// }

void Camera_trans(GLFWwindow* window, glm::fvec3 &Camera, glm::fvec3 &Target, glm::fvec3 &Up) {
    // use key board to move camera and center
    glm::fvec3 X = glm::normalize(glm::cross(Up, Target - Camera));
    if(glfwGetKey(window, GLFW_KEY_RIGHT) | glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        // Target = glm::translate(Target, glm::vec3(0.01f, 0.0f, 0.0f));
        Target += 0.1f * X;
        Camera += 0.1f * X;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) | glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(-0.01f, 0.0f, 0.0f));
        Target -= 0.1f * X;
        Camera -= 0.1f * X;
    }
    if(glfwGetKey(window, GLFW_KEY_UP) | glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(0.0f, 0.01f, 0.0f));
        Target -= 0.1f * glm::normalize(Up);
        Camera -= 0.1f * glm::normalize(Up);

    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) | glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(0.0f, -0.01f, 0.0f));
        Target += 0.1f * glm::normalize(Up);
        Camera += 0.1f * glm::normalize(Up);
    }
}

void Camera_move(glm::fvec3 &Camera, glm::fvec3 &Target, glm::fvec3 &Up) {
    // use mouse to move cameraPos
    const static float Speed = 1.0f;
    float theta = ImGui::GetIO().MouseDelta.x;
    float lambda = ImGui::GetIO().MouseDelta.y;
    
    theta = std::max(-10.0f, theta);
    theta = std::min(10.0f, theta);
    lambda = std::max(-10.0f, lambda);
    lambda = std::min(10.0f, lambda);
    if(abs(theta) > 1.5*abs(lambda)) lambda = 0;
    else if(1.5*abs(theta) < abs(lambda)) theta = 0;
    // lambda = std::max(-10, std::min(10, lambda) );
    // float theta = std::max(-10, std::min(10, (float)ImGui::GetIO().MouseDelta.x) );
    // float lamda = std::max(-10, std::min(10, (float)ImGui::GetIO().MouseDelta.y) );
    glm::fvec4 camTemp = glm::fvec4(Camera - Target, 0);


    glm::mat4 trans = glm::mat4(1.0f);



    theta = 0;
    lambda = 1;

    lambda = -lambda;



    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), 0.005f * theta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 0.005f * lambda, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // std::cout<<rot[0][0]<<" "<<rot[0][1]<<" "<<rot[0][2] <<" "<<'\n';
    // std::cout<<camTemp[0]<<" "<<camTemp[1]<<" "<<camTemp[2] <<" "<<'\n';
    camTemp = camTemp * rot;
    Up = glm::fvec4(Up, 0) * glm::rotate(glm::mat4(1.0f), 0.005f * theta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -0.005f * lambda, glm::vec3(1.0f, 0.0f, 0.0f));
    // std::cout<<camTemp[0]<<" "<<camTemp[1]<<" "<<camTemp[2] <<" "<<'\n';
    Camera = camTemp ;
    Camera = Camera + Target;
}

void Get_texture() {
    using namespace TextureImage;
    Texture& my = Texture::loadTexture("laugh", "C:\\Users\\27623\\Desktop\\Graphics\\Hand-cmake\\hand-graphics-homework-main\\Debug\\1.png");
    
    std::cerr<<"load completed\n";
    // my.bind(3);
}

int main(int argc, char *argv[]) {
    GLFWwindow *window;
    GLuint vertex_shader, fragment_shader, program;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__ // for macos
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(1000, 1000, "OpenGL output", NULL, NULL);


    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);

    //glfwSwapInterval(0);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;


    ImGui::StyleColorsDark();


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 100";
    ImGui_ImplOpenGL3_Init(glsl_version);


    if (glewInit() != GLEW_OK)
        exit(EXIT_FAILURE);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &SkeletalAnimation::vertex_shader_330, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &SkeletalAnimation::fragment_shader_330, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);


    Get_texture();



    int linkStatus;
    if (glGetProgramiv(program, GL_LINK_STATUS, &linkStatus), linkStatus == GL_FALSE)
        std::cout << "Error occured in glLinkProgram()" << std::endl;

    SkeletalMesh::Scene &sr = SkeletalMesh::Scene::loadScene("Hand", DATA_DIR"/Hand.fbx");

    if (&sr == &SkeletalMesh::Scene::error)
        std::cout << "Error occured in loadMesh()" << std::endl;

    sr.setShaderInput(program, "in_position", "in_texcoord", "in_normal", "in_bone_index", "in_bone_weight");

    float passed_time;
    SkeletalMesh::SkeletonModifier modifier;

    glEnable(GL_DEPTH_TEST);

    float metacarpals_angle = (M_PI / 4.0f);
    bool auto_rot = 1;

    float wave_angle = 0;
    bool wave_tag = 0;

    float scratch_angle = 0;
    float wave_angle2 = 0;
    bool scratch_tag = 0;
    bool point_tag = 0;
    const int max_epoch = 100;
    int cur_epoch = 0;

    float screen_rot = 0;

    // const float gesture_time_period = 7.0f;
    // float Begin_time = (float) glfwGetTime();

    auto calc_ratio = [&]() {

        return 0.5 - 0.5 * cos(1.0f * M_PI * cur_epoch / max_epoch);
    };

    glm::mat4 trans = glm::mat4(1.0f);

    // gesture_tag;
    bool OK_tag = 0;
    bool pistol_tag = 0;
    bool g_point_tag = 0;
    bool l_tag = 0;

    auto clear_gesture_tags = [&]() {
        wave_tag = wave_angle = 0;
        scratch_tag = 0;
        point_tag = 0;
        cur_epoch = 0;
        OK_tag = 0;
        pistol_tag = 0;
        g_point_tag = 0;
        l_tag = 0;
        // Begin_time = (float) glfwGetTime();
    };

    auto Init_gesture = [&]() {
        clear_gesture_tags();
        Gesture_default(modifier);
    };
    

    glm::fvec3 cameraPos = glm::fvec3(0.0f, .0f, -1.f);
    glm::fvec3 cameraTarget = glm::fvec3(0.0f, .0f, .0f);
    glm::fvec3 cameraUp = glm::fvec3(.0f, 2.0f, .0f);
    glm::fvec3 cameraFront = glm::fvec3(.0f, 0.0f, -1.0f);

    
    const glm::fvec3 originCameraPos = glm::fvec3(.0f, .0f, 10.f);
    const glm::fvec3 origincameraTarget = glm::fvec3(.0f, .0f, .0f);
    const glm::fvec3 origincameraUp = glm::fvec3(.0f, 1.0f, .0f);
    const glm::fvec3 origincameraFront = glm::fvec3(.0f, 0.0f, -1.0f);


    auto resetCamera = [&]() {
        cameraPos = originCameraPos;
        cameraTarget = origincameraTarget;
        cameraUp = origincameraUp;
        cameraFront = origincameraFront;
    };

    resetCamera();

    float CamZ = 0;
    while (!glfwWindowShouldClose(window)) {

        {
            
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Draw container
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }


        passed_time = (float) glfwGetTime();

        // --- You may edit below ---

        // Example: Rotate the hand
        // * turn around every 4 seconds

        // float metacarpals_angle = passed_time * (M_PI / 4.0f);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // if(ImGui :: Button("Wave action\n")) {
        //     wave_tag ^= 1;
        // }
        if(wave_tag) {
            Gesture_default(modifier);
            clear_gesture_tags();
            wave_tag = 1;
            float period = 2.4f;
            float time_in_period = fmod(passed_time, period);
            // * angle: 0 -> PI/3 -> 0
            // wave_angle = abs(time_in_period - period*0.5) / (period*0.5) * (M_PI * 0.25) - (M_PI * 0.125);
            wave_angle = cos(2 * M_PI * time_in_period / period) * (M_PI * 0.125);
        }

        // ImGui :: SliderAngle("waving", &wave_angle, -180.0f, 180.0f);

        // wave first
        modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), screen_rot, glm::fvec3(0.0, 0.0, 1.0)) * glm::rotate(glm::identity<glm::mat4>(), wave_angle, glm::fvec3(0.0, 1.0, 0.0));


        // modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), metacarpals_angle, glm::fvec3(0.0, 1.0, 0.0));

        /**********************************************************************************\
        *
        * To animate fingers, modify modifier["HAND_SECTION"] each frame,
        * where HAND_SECTION can only be one of the bone names in the Hand's Hierarchy.
        *
        * A virtual hand's structure is like this: (slightly DIFFERENT from the real world)
        *    5432 1
        *    ....        1 = thumb           . = fingertip
        *    |||| .      2 = index finger    | = distal phalange
        *    $$$$ |      3 = middle finger   $ = intermediate phalange
        *    #### $      4 = ring finger     # = proximal phalange
        *    OOOO#       5 = pinky           O = metacarpals
        *     OOO
        * (Hand in the real world -> https://en.wikipedia.org/wiki/Hand)
        *
        * From the structure we can infer the Hand's Hierarchy:
        *	- metacarpals
        *		- thumb_proximal_phalange
        *			- thumb_intermediate_phalange
        *				- thumb_distal_phalange
        *					- thumb_fingertip
        *		- index_proximal_phalange
        *			- index_intermediate_phalange
        *				- index_distal_phalange
        *					- index_fingertip
        *		- middle_proximal_phalange
        *			- middle_intermediate_phalange
        *				- middle_distal_phalange
        *					- middle_fingertip
        *		- ring_proximal_phalange
        *			- ring_intermediate_phalange
        *				- ring_distal_phalange
        *					- ring_fingertip
        *		- pinky_proximal_phalange
        *			- pinky_intermediate_phalange
        *				- pinky_distal_phalange
        *					- pinky_fingertip
        *
        * Notice that modifier["HAND_SECTION"] is a local transformation matrix,
        * where (1, 0, 0) is the bone's direction, and apparently (0, 1, 0) / (0, 0, 1)
        * is perpendicular to the bone.
        * Particularly, (0, 0, 1) is the rotation axis of the nearer joint.
        *
        \**********************************************************************************/

        // Example: Animate the index finger
        // * period = 2.4 seconds
        // float period = 2.4f;
        // float time_in_period = fmod(passed_time, period);
        // * angle: 0 -> PI/3 -> 0
        // float thumb_angle = abs(time_in_period / (period * 0.5f) - 1.0f) * (M_PI / 3.0);
     
        // if(ImGui::Button("scratch")) {
        //     Init_gesture();
        //     scratch_tag ^= 1;
        // }
        if(scratch_tag) {
            scratch_tag = 1;
            ImGui :: SliderAngle("scratch_angle", &scratch_angle, 0.0f, 45.0f);
            scratch(modifier, scratch_angle);
        }

    
        // if(ImGui::Button("gesture default")) {
        //     Init_gesture();
        //     Gesture_default(modifier);
        // }

        // if(ImGui::Button("gesture oK")) {
        //     Init_gesture();
        //     OK_tag = 1;
        // }


        if(OK_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_OK(modifier, calc_ratio());
        }

        // if(ImGui::Button("gesture pistol")) {
        //     Init_gesture();
        //     pistol_tag = 1;
        // }
        if(pistol_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_pistol(modifier, calc_ratio());
            wave_angle = calc_ratio() * M_PI / 2.0f;
        }


        if(ImGui::Button("gesture Love")) {
            Init_gesture();
            l_tag = 1;
        }
        if(l_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_L(modifier, calc_ratio());
            wave_angle = calc_ratio() * M_PI / 3.0f;
            screen_rot = -calc_ratio() * M_PI / 2.5f;
            metacarpals_angle = calc_ratio() * M_PI * 1.45f;
        }




        // if(ImGui::Button("gesture point")) {
        //     Init_gesture();
        //     point_tag = 1;
        // }
        if(point_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_point(modifier, calc_ratio());
            wave_angle = calc_ratio() * M_PI / 2.0f;
            float rot_z = calc_ratio() *  M_PI / 2.0f;
            modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), rot_z, glm::fvec3(0.0, 0.0, 1.0)) * modifier["metacarpals"];
        }

        // rotation
        // ImGui::Text("Manual rotation or autorotation");
        // if(ImGui::Button("autorotation ")) auto_rot ^= 1;
        // if(auto_rot) metacarpals_angle = passed_time * (M_PI / 4.0f);
        // else ImGui :: SliderAngle("metacarpals_angle", &metacarpals_angle, 0.0f, 360.f);


        // Camera_trans(window, trans);
        Camera_trans(window, cameraPos, cameraTarget, cameraUp);
        // if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        //     Camera_move(cameraPos, cameraTarget, cameraUp);
        // }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
            Camera_move(cameraPos, cameraTarget, cameraUp);
        }

        modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), metacarpals_angle, glm::fvec3(1.0, 0.0, 0.0)) * modifier["metacarpals"];
        
        // --- You may edit above ---


        // ImGui::Text("\n--Move by arrow keys or W A S D");
        // ImGui::Text("--Zoom through the mouse wheel");
        // ImGui::Text("--Press the reposition key \nto return to the default position ");
        
        // ImGui::Text("cameraPos:    (%.2f, %.2f, %.2f)\ncameraTarget: (%.2f, %.2f, %.2f)\n",cameraPos[0], cameraPos[1],cameraPos[2], cameraTarget[0], cameraTarget[1], cameraTarget[2]);
        // ImGui::Text("cameraUp:   (%.2f, %.2f, %.2f)\n",cameraUp[0], cameraUp[1],cameraUp[2]);
        
        // if(ImGui::Button("Reposition")) {
        //     trans = glm::mat4(1.0f);
        //     scale = glm::mat4(1.0f);
        //     cur_scr = 20;

        //     resetCamera();
        // }

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glClearColor(0.5, 0.5, 0.5, 1.0);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glUseProgram(program);



        // glm::fvec3 cameraPos = glm::fvec3(.0f, .0f, -1.f);

        // glm::fvec3 cameraTarget = glm::fvec3(.0f, .0f, .0f);
        // glm::fvec3 cameraUp = glm::fvec3(.0f, 1.0f, .0f);

        // ImGui::SliderAngle("233", CamZ, 0, 360.0f);
        // cameraPos = glm::fvec3(0.0f, 20.0f, -1.0f);

        glm::fmat4 mvp = 
                        // trans
                        // *
                        // scale
                        // *
                        glm::ortho(-12.5f * ratio, 12.5f * ratio, -5.f, 20.f, -20.f, 20.f)
                         *
                         glm::lookAt(cameraPos, cameraTarget, cameraUp)
                        //  glm::lookAt(glm::fvec3(.0f, .0f, -1.f), glm::fvec3(.0f, .0f, .0f), glm::fvec3(.0f, 1.f, .0f))
                        ;


        glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE, (const GLfloat *) &mvp);
        glUniform1i(glGetUniformLocation(program, "u_diffuse"), SCENE_RESOURCE_SHADER_DIFFUSE_CHANNEL);

        SkeletalMesh::Scene::SkeletonTransf bonesTransf;

        sr.getSkeletonTransform(bonesTransf, modifier);

        if (!bonesTransf.empty())
            glUniformMatrix4fv(glGetUniformLocation(program, "u_bone_transf"), bonesTransf.size(), GL_FALSE,
                               (float *) bonesTransf.data());
        

        sr.render();
        ImGui::Render(); // rendering
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();

    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    SkeletalMesh::Scene::unloadScene("Hand");

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}