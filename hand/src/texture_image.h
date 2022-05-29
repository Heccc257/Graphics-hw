// Simple Texture Image Loader & Renderer
// Author: Yi Kangrui <yikangrui@pku.edu.cn>

#pragma once

#include <iostream>

#include <vector>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>

#include "gl_env.h"

#include <stb_image.h>

GLuint VBO, VAO, EBO;
namespace TextureImage {
    class Texture {
    public:
        typedef std::map<std::string, Texture *> Name2Texture;
        static Name2Texture allTexture;
        static Texture error;

    private:
        bool available;
        std::string name;
        std::string filename;
        int width;
        int height;
        GLuint tex;

        // Forbid calling any constructor outside
        Texture(const Texture &_copy)
                : Texture() {}

        Texture()
                : available(false), name(), filename(), width(0), height(0), tex(0) {}

        virtual ~Texture() { clear(); }

    public:
        void clear() {
            available = false;
            name = std::string();
            filename = std::string();
            glDeleteTextures(1, &tex);
            tex = 0;
        }

        static std::string testAllSuffix(std::string no_suffix_name) {
            const int support_suffix_num = 4;
            const std::string support_suffix[support_suffix_num] = {
                    ".bmp",
                    ".jpg",
                    ".png",
                    ".tga"
            };

            for (int i = 0; i < support_suffix_num; i++) {
                std::string try_filename = no_suffix_name + support_suffix[i];
                FILE *ftest = fopen(try_filename.c_str(), "r");
                if (ftest) {
                    fclose(ftest);
                    return try_filename;
                }
            }
            return std::string();
        }

        static Texture &loadTexture(std::string _name, std::string _filename = std::string()) {
            GLenum gl_error_code = GL_NO_ERROR;
            if ((gl_error_code = glGetError()) != GL_NO_ERROR) {
                const GLubyte *errString = glewGetErrorString(gl_error_code);
                std::cout << "ERROR before loadTexture():" << std::endl;
                std::cout << errString << std::endl;
            }
            std::cout << _name << "<>" << _filename << std::endl;
            if (_filename.empty() || _filename == "") {
                _filename = testAllSuffix(_name);
                if (_filename.empty()) return error;
            }
            FILE *fi = fopen(_filename.c_str(), "r");
            if (fi == NULL) return error;
            fclose(fi);

            std::pair<Name2Texture::iterator, bool> insertion =
                    allTexture.insert(Name2Texture::value_type(_name, new Texture()));
            Texture &target = *(insertion.first->second);
            if (!insertion.second) {
                if (target.filename == _filename && target.available) {
                    return target;
                } else {
                    target.clear();
                }
            }

            target.name = _name;
            target.filename = _filename;

            stbi_set_flip_vertically_on_load(true);
            int width, height, channels;
            unsigned char *data =
                    stbi_load(_filename.c_str(), &width, &height, &channels, 0);
            if (!data) {
                return error;
            }

            GLenum format = GL_RGBA;
            if (channels == 1) {
                format = GL_R;
            }
            if (channels == 2) {
                format = GL_RG;
            }
            if (channels == 3) {
                format = GL_RGB;
            }


// copied
            GLfloat vertices[] = {
                // Positions          // Colors           // Texture Coords
                0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
                0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
                -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
                -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left 
            };
            GLuint indices[] = {  // Note that we start from 0!
                0, 1, 3, // First Triangle
                1, 2, 3  // Second Triangle
            };
            
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            // Color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);
            // TexCoord attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
            glEnableVertexAttribArray(2);

            glBindVertexArray(0); // Unbind VAO
    
// copied


            glGenTextures(1, &target.tex);
            glBindTexture(GL_TEXTURE_2D, target.tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, target.width, target.height,
                         0, format, GL_UNSIGNED_BYTE, data);
            // std::cerr<<"Bind\n";
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(data);

            if ((gl_error_code = glGetError()) != GL_NO_ERROR) {
                const GLubyte *errString = glewGetErrorString(gl_error_code);
                std::cout << "ERROR in loadTexture():" << std::endl;
                std::cout << errString << std::endl;
                return error;
            }

            target.available = true;
            return target;
        }

        static bool unloadTexture(std::string _name) {
            return allTexture.erase(_name) != 0;
        }

        static Texture &getTexture(const std::string &_name) {
            Name2Texture::iterator find_result = allTexture.find(_name);
            if (find_result == allTexture.end()) return error;
            return *(find_result->second);
        }

        bool bind(GLenum textureChannel) const {
            if (!available) return false;
            std::cerr<<"bind233\n";
            glActiveTexture(GL_TEXTURE0 + textureChannel);
            glBindTexture(GL_TEXTURE_2D, tex);
            return true;
        }
    };

    Texture::Name2Texture Texture::allTexture;
    Texture Texture::error;
}