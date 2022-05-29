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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include<cmath>
#include<map>
#include<fstream>
#include<imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "light.h"

// #include "texture.h"
// #include "shader.h"
// #include "particle_generator.h"
// #include "resource_manager.h"
// #include "sprite_renderer.h"
// #include "game_object.h"


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

            "out vec3 FragPos;\n"
            "out vec3 Normal;\n"

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

            "    Normal = mat3(transpose(inverse(bone_transform))) * in_normal;\n"
            "    FragPos = vec3(bone_transform * vec4(in_position, 1.0));\n"

            "}\n";

    const char *fragment_shader_330 =
            "#version 330 core\n"
            "uniform sampler2D u_diffuse;\n"
            
            "uniform vec4 FragColor;\n"
            "uniform vec3 lightPos;\n"
            "uniform vec3 lightColor;\n"
            "uniform vec3 lightShootDir;\n"
            "uniform vec3 viewPos;\n"

            "in vec2 pass_texcoord;\n"
            "in vec3 Normal;\n"
            "in vec3 FragPos;\n"

            "out vec4 out_color;\n"
            "void main() {\n"

            "    vec3 objectColor = vec3(pass_texcoord, 0.0);\n"
            "    float ambientStrength = 0.5;\n"
            "    vec3 ambient = ambientStrength * lightColor;\n"
                // diffuse 
            "    vec3 norm = normalize(Normal);\n"
            "    vec3 lightDir = normalize(lightPos - FragPos);\n"
            "    float diff = max(dot(norm, lightDir), 0.0);\n"
            "    vec3 diffuse = diff * lightColor;\n"
                // specular
            "    float specularStrength = 0.7;\n"
            "    vec3 viewDir = normalize(viewPos - FragPos);\n"
            "    vec3 reflectDir = reflect(-lightDir, norm);  \n"
            "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);\n"
            "    vec3 specular = specularStrength * spec * lightColor;  \n"
            
            "    float shootStrength = max(dot(normalize(lightShootDir), lightDir), 0.0);\n"
            "    diffuse = shootStrength * diffuse;\n"
            "    specular = shootStrength * specular;\n"
            "    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
            "    vec4 FragColor = vec4(result, 1.0);\n"

            #ifdef DIFFUSE_TEXTURE_MAPPING
            "    out_color = vec4(texture(u_diffuse, pass_texcoord).xyz, 1.0);\n"
            #else
            // "    out_color = vec4(pass_texcoord, 0.0, 1.0);\n"
            "    out_color = FragColor;\n"
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

float camera_forward = 0;
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    cur_scr += yoffset;


    camera_forward = yoffset;

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




char ShaderCode[1<<15];
char * load_shaderFile(const char *filePath) {
    std::cerr<<"OK\n";
    std::cerr<<"file = "<<filePath<<"\n";
    std::ifstream ifile;
    memset(ShaderCode, '\0', sizeof(ShaderCode));
    ifile.open(filePath, std::ios::in);
    if(!ifile.is_open()) {
        std::cerr<<"fail\n";
    } else {

        int offset = 0;
        while(ifile>>ShaderCode+offset) {
            offset = strlen(ShaderCode);
            strcpy(ShaderCode+offset, "\n");
            offset++;
        }
    }
    // std::cerr<<ShaderCode<<"\n";
    return &ShaderCode[0];
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


/*------------------------------------particles--------------------------------*/
    // ResourceManager::LoadShader("..\\..\\..\\src\\shaders\\sprite.vs", "..\\..\\..\\src\\shaders\\sprite.frag", nullptr, "sprite");
    // ResourceManager::LoadShader("..\\..\\..\\src\\shaders\\particle.vs", "..\\..\\..\\src\\shaders\\particle.frag", nullptr, "particle");

    // Configure shaders
    // float Width = 20.0f, Height = 20.0f;
    // glm::mat4 projection = glm::ortho(-12.5f, 12.5f, -5.f, 20.f, -20.f, 20.f);
    // ResourceManager::GetShader("sprite").Use().SetInteger("sprite", 0);
    // ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    // ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
    // Load textures
    // ResourceManager::LoadTexture("..\\..\\..\\src\\textures\\background.jpg", GL_FALSE, "background");
    // ResourceManager::LoadTexture("textures\\awesomeface.png", GL_TRUE, "face");
    // ResourceManager::LoadTexture("textures\\block.png", GL_FALSE, "block");
    // ResourceManager::LoadTexture("textures\\block_solid.png", GL_FALSE, "block_solid");
    // ResourceManager::LoadTexture("textures\\paddle.png", GL_TRUE, "paddle");
    // ResourceManager::LoadTexture("..\\..\\..\\src\\textures\\particle.png", GL_TRUE, "particle");
    // Set render-specific controls
    // SpriteRenderer *Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // ParticleGenerator *Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);

/*-----------------------------------------------------------------------------*/




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

    float metacarpals_angle = (M_PI / 1.0f);
    bool auto_rot = 0;

    float wave_angle = 0;
    bool wave_tag = 0;

    float scratch_angle = 0;
    bool scratch_tag = 0;

    bool point_tag = 0;

    const int max_epoch = 100;
    int cur_epoch = 0;

    // const float gesture_time_period = 7.0f;
    // float Begin_time = (float) glfwGetTime();

    auto calc_ratio = [&](int &cur_epoch) {

        return 0.5 - 0.5 * cos(1.0f * M_PI * cur_epoch / max_epoch);
    };

    glm::mat4 trans = glm::mat4(1.0f);

    // gesture_tag;
    bool OK_tag = 0;
    bool pistol_tag = 0;
    bool g_point_tag = 0;

    auto clear_gesture_tags = [&]() {
        cur_epoch = 0;

        wave_tag = wave_angle = 0;
        scratch_tag = 0;
        point_tag = 0;
        OK_tag = 0;
        pistol_tag = 0;
        g_point_tag = 0;
        // Begin_time = (float) glfwGetTime();
    };

    auto Init_gesture = [&]() {
        clear_gesture_tags();
        Gesture_default(modifier);
    };
    

    bool camera_tag = 0;
    int camera_move_epoch = 0;
    float pitch = 0;
    float yaw = 0;
    glm::fvec3 cameraPos = glm::fvec3(0.0f, .0f, 5.f);
    glm::fvec3 cameraTarget = glm::fvec3(0.0f, .0f, .0f);
    glm::fvec3 cameraUp = glm::fvec3(.0f, 2.0f, .0f);
    glm::fvec3 cameraFront = glm::fvec3(.0f, 0.0f, -1.0f);
    
    
    // const glm::fvec3 originCameraPos = glm::fvec3(.0f, -5.f*sqrt(2), 5.f*sqrt(2));
    const glm::fvec3 originCameraPos = glm::fvec3(.0f, .0f, 5.f);
    const glm::fvec3 origincameraTarget = glm::fvec3(.0f, .0f, .0f);
    const glm::fvec3 origincameraUp = glm::fvec3(.0f, 1.0f, .0f);
    const glm::fvec3 origincameraFront = glm::fvec3(.0f, 0.0f, -1.0f);


    auto resetCamera = [&]() {
        cameraPos = originCameraPos;
        cameraTarget = origincameraTarget;
        cameraUp = origincameraUp;
        cameraFront = origincameraFront;
        camera_move_epoch = 0;
        pitch = 0;
        yaw = 0;

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


        if(ImGui :: Button("Wave action\n")) {
            wave_tag ^= 1;
        }
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

        ImGui :: SliderAngle("waving", &wave_angle, -180.0f, 180.0f);

        // wave first
        modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), wave_angle, glm::fvec3(0.0, 1.0, 0.0));


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
     
        if(ImGui::Button("scratch")) {
            Init_gesture();
            scratch_tag ^= 1;
        }
        if(scratch_tag) {
            scratch_tag = 1;
            ImGui :: SliderAngle("scratch_angle", &scratch_angle, 0.0f, 45.0f);
            scratch(modifier, scratch_angle);
        }


        if(ImGui::Button("gesture default")) {
            Init_gesture();
            Gesture_default(modifier);
        }

        if(ImGui::Button("gesture oK")) {
            Init_gesture();
            OK_tag = 1;
        }


        if(OK_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_OK(modifier, calc_ratio(cur_epoch));
        }

        if(ImGui::Button("gesture pistol")) {
            Init_gesture();
            pistol_tag = 1;
        }
        if(pistol_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_pistol(modifier, calc_ratio(cur_epoch));
            wave_angle = calc_ratio(cur_epoch) * M_PI / 2.0f;
        }




        if(ImGui::Button("gesture point")) {
            Init_gesture();
            point_tag = 1;
        }
        if(point_tag) {
            if(cur_epoch < max_epoch) cur_epoch ++ ;
            Gesture_point(modifier, calc_ratio(cur_epoch));
            wave_angle = calc_ratio(cur_epoch) * M_PI / 2.0f;
            float rot_z = calc_ratio(cur_epoch) *  M_PI / 2.0f;
            modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), rot_z, glm::fvec3(0.0, 0.0, 1.0)) * modifier["metacarpals"];
        }

        // rotation
        ImGui::Text("Manual rotation or autorotation");
        if(ImGui::Button("autorotation ")) auto_rot ^= 1;
        if(auto_rot) metacarpals_angle = passed_time * (M_PI / 4.0f);
        else ImGui :: SliderAngle("metacarpals_angle", &metacarpals_angle, 0.0f, 360.f);




        
        if(ImGui::Button("camera from a to b")) {
            resetCamera();
            camera_tag ^= 1;
        }

        auto rota2b = [&](glm::fvec3 a, glm::fvec3 b, float ratio) {
            a = glm::normalize(a);
            b = glm::normalize(b);
            float RotationAngle = acos(glm::dot(a, b));
            glm::fvec3 RotationAxis = glm::cross(b, a);
            return glm::rotate(glm::fmat4(1.f), ratio * RotationAngle, RotationAxis);
        };
        if(camera_tag) {
            const static glm::fvec3 a = originCameraPos;
            const static glm::fvec3 b = glm::fvec3(2.f, 2.f, 2.f);
            const static glm::fvec3 bUp = glm::normalize(glm::fvec3(-1.f, 2.f, -1.f));
            const static glm::fvec3 RotationAxis = glm::normalize(glm::cross(a, b));


            // float RotationAngle = acos(glm::dot(glm::normalize(a), glm::normalize(b)));
            // std::cout<<"angle="<<RotationAngle<<"\n";
            // std::cout<<"axis "<<RotationAxis.x<<" "<<RotationAxis.y<<" "<<RotationAxis.z<<"\n";
            // glm::fmat4 Quat_rot = glm::rotate(glm::fmat4(1.f), ratio * RotationAngle, RotationAxis);
            if(camera_move_epoch < max_epoch) camera_move_epoch ++;
            float ratio = calc_ratio(camera_move_epoch);
            glm::fmat4 Quat_rot = rota2b(a, b, ratio);
            cameraPos = glm::fvec4(originCameraPos, 0) * Quat_rot;
            cameraUp = glm::fvec4(cameraUp, 0) * rota2b(cameraUp, bUp, ratio);
            cameraFront = glm::normalize(origincameraTarget - cameraPos);
        }

        Camera_trans(window, cameraPos, cameraTarget, cameraUp);

        cameraPos += camera_forward * cameraFront ;
        camera_forward = 0;

        if(!camera_tag) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
                View_move(cameraPos, cameraTarget, cameraUp, pitch, yaw);
                cameraFront = origincameraFront;
                cameraUp = origincameraUp;
                glm::fvec3 X = glm::normalize(glm::cross(cameraFront, cameraUp));
                glm::mat4 Euler = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), X) * glm::rotate(glm::mat4(1.0f), glm::radians(yaw), cameraUp);
                cameraFront = glm::fvec4(cameraFront, 0) * Euler;
                cameraUp = glm::fvec4(cameraUp, 0) * Euler;
                cameraTarget = cameraPos + cameraFront;
            }
        }

        modifier["metacarpals"] = glm::rotate(glm::identity<glm::mat4>(), metacarpals_angle, glm::fvec3(1.0, 0.0, 0.0)) * modifier["metacarpals"];
        
        // --- You may edit above ---
/*------------------------------------Light------------------------------------*/
        // Shader lightCubeShader("shaders\\light_cube.vs", "shaders\\light_cube.fs");
        ImGui::Text("---------light---------\n");
        load_light(program);
        Move_light();
        // ImGui::End();
        ImGui::Text("-----------------------\n");
/*-----------------------------------------------------------------------------*/


        // ImGui::Text("\n--Move by arrow keys or W A S D");
        // ImGui::Text("--Zoom through the mouse wheel");
        // ImGui::Text("--Press the reposition key \nto return to the default position ");
        
        ImGui::Text("cameraPos:    (%.2f, %.2f, %.2f)\ncameraTarget: (%.2f, %.2f, %.2f)\n",cameraPos[0], cameraPos[1],cameraPos[2], cameraTarget[0], cameraTarget[1], cameraTarget[2]);
        ImGui::Text("cameraUp:   (%.2f, %.2f, %.2f)\n",cameraUp[0], cameraUp[1],cameraUp[2]);
        ImGui::Text("(pitch, yaw):   (%.5f, %.5f)\n",pitch, yaw);
        
        if(ImGui::Button("Reposition")) {
            trans = glm::mat4(1.0f);
            scale = glm::mat4(1.0f);
            cur_scr = 20;

            resetCamera();
            camera_tag = 0;
        }

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
                        //  glm::lookAt(cameraPos, cameraTarget, cameraUp)
                         glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)
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