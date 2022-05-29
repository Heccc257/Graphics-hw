

glm::vec3 lightPos(3.5f, 0.5f, 2.5f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 lightShootDir = -lightPos;

void Move_light() {
    // float v[3];
    // ImGui :: SliderFloat3("light position x", v, -15.0f, 15.0f);
    ImGui :: SliderFloat("light  x", &lightPos.x, -5.0f, 5.0f, "%.3f", 0);
    ImGui :: SliderFloat("light  y", &lightPos.y, -3.0f, 3.0f, "%.3f", 0);
    ImGui :: SliderFloat("light  z", &lightPos.z, 1.0f, 4.0f, "%.3f", 0);
    ImGui :: SliderFloat("light R", &lightColor.x, -0.0, 1.0f, "%.3f", 0);
    ImGui :: SliderFloat("light G", &lightColor.y, -0.0f, 1.0f, "%.3f", 0);
    ImGui :: SliderFloat("light B", &lightColor.z, -0.0f, 1.0f, "%.3f", 0);
}

void load_light(GLuint &shader_prog) {
    // light always shoots (0,0,0)
    lightShootDir = -lightPos;
    glUniform3f(glGetUniformLocation(shader_prog, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shader_prog, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
    glUniform3f(glGetUniformLocation(shader_prog, "lightShootDir"), lightShootDir.x, lightShootDir.y, lightShootDir.z);
}
