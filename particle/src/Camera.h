
void View_move(glm::fvec3 &Camera, glm::fvec3 &Target, glm::fvec3 &Up, float &pitch, float &yaw) {
    const static float maxPitch = 65.f;
    const static float maxYaw = 65.f;
    const static float minYaw = -65.f;
    const static float minPitch = -65.f;


    // use mouse to move cameraPos
    const static float Speed = 0.3f;
    float theta = ImGui::GetIO().MouseDelta.x;
    float lambda = ImGui::GetIO().MouseDelta.y;

    

    theta = std::max(-10.0f, theta);
    theta = std::min(10.0f, theta);
    lambda = std::max(-10.0f, lambda);
    lambda = std::min(10.0f, lambda);
    if(abs(theta) > 1.5*abs(lambda)) lambda = 0;
    else if(1.5*abs(theta) < abs(lambda)) theta = 0;

    yaw += theta * Speed;
    pitch += lambda * Speed;
    yaw = std::min(yaw, maxYaw);
    yaw = std::max(yaw, minYaw);
    pitch = std::min(pitch, maxPitch);
    pitch = std::max(pitch, minPitch);
}


void Camera_trans(GLFWwindow* window, glm::fvec3 &Camera, glm::fvec3 &Target, glm::fvec3 &Up) {
    // use key board to move camera and center
    glm::fvec3 X = glm::normalize(glm::cross(Up, Target - Camera));
    if(glfwGetKey(window, GLFW_KEY_RIGHT) | glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        // Target = glm::translate(Target, glm::vec3(0.01f, 0.0f, 0.0f));
        Target -= 0.1f * X;
        Camera -= 0.1f * X;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) | glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(-0.01f, 0.0f, 0.0f));
        Target += 0.1f * X;
        Camera += 0.1f * X;
    }
    if(glfwGetKey(window, GLFW_KEY_UP) | glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(0.0f, 0.01f, 0.0f));
        Target += 0.1f * glm::normalize(Up);
        Camera += 0.1f * glm::normalize(Up);

    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) | glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        // trans = glm::translate(trans, glm::vec3(0.0f, -0.01f, 0.0f));
        Target -= 0.1f * glm::normalize(Up);
        Camera -= 0.1f * glm::normalize(Up);
    }
}


struct quat {
    double w, x, y, z;
    quat() {}
    quat(double _w, double _x, double _y, double _z):w(_w),x(_x),y(_y),z(_z) {}
    quat(glm::fvec3 RotationAxis, double RotationAngle) {
        x = RotationAxis.x * sin(RotationAngle / 2);
        y = RotationAxis.y * sin(RotationAngle / 2);
        z = RotationAxis.z * sin(RotationAngle / 2);
        w = cos(RotationAngle / 2);
    }
    glm::fvec4 vec4() {
        return glm::fvec4(x, y, z, w);
    }
    bool equals(quat& a) {
        return (glm::dot(vec4(), a.vec4()) == 1);
    }
};