/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "particle_generator.h"
#include <iostream>
#include <cmath>
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif


const float rmax = 0x7fff;
ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, GLuint amount)
    : shader(shader), texture(texture), amount(amount)
{
    this->init();
}

glm::vec3 MylightPos, MylightShootDir, MylightColor;
glm::vec3 MyviewPos, MyviewFront;

const float initial_speed = 950;
float grav = 10;

void ParticleGenerator::Update(GLfloat dt, glm::vec3 Launcher, GLuint newParticles, glm::vec2 offset)
{
    if(ImGui::Button("action 1")) {
        action1 = 4000;
        action2 = 0;
        action3 = 0;
    }
    if(action1>0) action1--;
    if(ImGui::Button("action 2")) {
        action1 = 0;
        action2 = 4000;
        action3 = 0;
    }
    if(action2>0) action2--;
    if(ImGui::Button("action 3")) {
        action1 = 0;
        action2 = 0;
        action3 = 4000;
    }
    if(action3>0) action3--;

    ImGui::SliderFloat("acceleration of gravity", &grav, 0.0, 12.0);
    // Add new particles 
    for (GLuint i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        if(unusedParticle < 0) break;
        this->respawnParticle(this->particles[unusedParticle], Launcher, offset);
    }
    // Update all particles
    for (GLuint i = 0; i < this->amount; ++i)
    {
        Particle &p = this->particles[i];
        p.Life -= dt; // reduce life
        if (p.Life > 0.0f)
        {	// particle is alive, thus update
            p.Position -= p.Velocity * dt; 
            // p.Color -= glm::vec4(0.0, 0.0, 0.0, 0.6*dt);

            p.Velocity.y += grav;
            if(p.Life > 0.95) {
                p.Velocity.x *= 0.994987;
                p.Velocity.y *= 0.994987;
            }
            if(p.Life<0.8 && p.Position.y < 25) {
                p.Velocity.y = -0.4 * p.Velocity.y;
                // p.Color -= glm::vec4(0.3) * p.Color;
                // p.Color += glm::vec4(0.5*p.Color, 0.6*dt, 0.6*dt, 0.0);
            }
            if(p.Position.x < 0 || p.Position.x > 300) {
                p.Velocity.x = -0.4 * p.Velocity.x;
            }
        }

    }
}

// Render all particles
void ParticleGenerator::Draw()
{
    // Use additive blending to give it a 'glow' effect
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    this->shader.Use();
    
    this->shader.SetVector3f("Normal", glm::vec3(0.0f, 0.0, -1.0f));
    this->shader.SetVector3f("lightPos", MylightPos);
    this->shader.SetVector3f("lightShootDir", MylightShootDir);
    this->shader.SetVector3f("lightColor", MylightColor);
    this->shader.SetVector3f("viewPos", MyviewPos);
    this->shader.SetVector3f("viewFront", MyviewFront);
    this->shader.SetVector3f("Normal", glm::vec3(0.0, 0.0, 1.0));
    for (Particle particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {
            this->shader.SetVector2f("offset", particle.Position);
            this->shader.SetVector4f("color", particle.Color);
            this->shader.SetFloat("Life", particle.Life);
            this->shader.SetFloat("Z", rand()/rmax*0.5-0.25);
            this->texture.Bind();
            glBindVertexArray(this->VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // Don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

struct Circle{
    float r;
    float w;
    float theta;
    glm::vec4 obj_color;
    Circle(float _r, float _w, float _theta, glm::vec4 _color) :
        r(_r), w(_w), theta(_theta), obj_color(_color) {}
};
std::vector<Circle>circles;
void ParticleGenerator::init()
{
    srand(time(0));
    // Set up mesh and attribute properties
    GLuint VBO;
    GLfloat particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);
    // Fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // Set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    // Create this->amount default particle instances
    this->particles.clear();
    for (GLuint i = 0; i < this->amount; ++i)
        this->particles.push_back(Particle());
    
    // circles.push_back
}

// Stores the index of the last particle used (for quick access to next dead particle)
GLuint lastUsedParticle = 0;
GLuint ParticleGenerator::firstUnusedParticle()
{
    // // First search from last used particle, this will usually return almost instantly
    // for (GLuint i = lastUsedParticle; i < this->amount; ++i){
    //     if (this->particles[i].Life <= 0.0f){
    //         lastUsedParticle = i;
    //         return i;
    //     }
    // }
    // // Otherwise, do a linear search
    // for (GLuint i = 0; i < lastUsedParticle; ++i){
    //     if (this->particles[i].Life <= 0.0f){
    //         lastUsedParticle = i;
    //         return i;
    //     }
    // }

    // // All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    // lastUsedParticle = 0;
    if(this->particles[lastUsedParticle].Life <= 0.0f) {
        int tem = lastUsedParticle;
        lastUsedParticle = (lastUsedParticle + 1) % this->amount;
        return tem;
    } else return -1;
    return 0;
}



void ParticleGenerator::respawnParticle(Particle &particle, glm::vec3 Launcher, glm::vec2 offset)
{



    GLfloat random = ((rand() % 100) - 50) / 10.0f;
    GLfloat rColor = 0.7 + ((rand() / rmax) * 0.3);
    // glm::vec2 randOffset = glm::vec2(rand()/rmax*30, rand()/rmax*30) - glm::vec2(15, 15);
    glm::vec2 randOffset = glm::vec2(0, rand()/rmax*10);
    int r3 = rand()%23;
    if(r3<3) r3=0;
    else if(r3<8) r3=1;
    else if(r3<15) r3=2;
    else if(r3<20) r3=3;
    else if(r3<23) r3=4;

    if(action3>0) {
        int epoch = action3/100;
        r3 = epoch*2%5;
    }

    randOffset.x = 50*(r3-2) + rand()/rmax*6 - 3;

    particle.Position = glm::vec2(Launcher.x, Launcher.y) + randOffset;
    

    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    // particle.Color = glm::vec4(1.0, 1.0, 1.0, 1.0f);
    particle.Life = 1.0f;

    float speed = (rand()/rmax) * 80 + initial_speed;
    speed -= abs(r3-2) * 80;

    if(action1>0) {
        if(action1%100 < 50) speed *= 0.8;
    }
    // float angle = -M_PI/2 + (rand()%103 / 100.0f - M_PI/6.0f);
    float angle = M_PI / 2 + M_PI*0.05 *(rand()/rmax) - M_PI*0.025;

    if(action2>0) {
        int rd = action2 % 100;
        angle += cos(2*M_PI * action2/100) * M_PI * 0.2;
    }

    particle.Velocity.x = speed * cos(angle);
    particle.Velocity.y = - (speed * sin(angle) + 50);
    // particle.Velocity.x = ((rand() % 1000) - 500) / 5.0f;
    // particle.Velocity.y = - ((rand() % 500) + 300) / 5.0f;
}

void ParticleGenerator::loadLight(glm::vec3 lightPos, glm::vec3 lightShootDir, glm::vec3 lightColor) {
    // std::cerr<<lightColor.x<<" "<<lightColor.y<<" "<<lightColor.z<<"\n";
    MylightPos = lightPos;
    MylightShootDir = lightShootDir;
    MylightColor = lightColor;

}
void ParticleGenerator::loadCamera(glm::vec3 viewPos, glm::vec3 viewFront) {
    MyviewPos = viewPos;
    MyviewFront = viewFront;
}