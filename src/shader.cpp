// terrain_gl
// @codedstructure 2023

#include <GL/glew.h>
#include <fstream>
#include <iostream>
#include <string>
#include "shader.h"

Shader::Shader(unsigned int shader_type, const char* filename) :
        m_Compiled(0) {
    basic_ifstream<char> fShaderFile(filename);
    if (!fShaderFile.is_open()) {
        std::cerr << "Cannot open shader " << filename << "\n";
        //return 0;
    } else {
        m_ShaderText = basic_string<char>(
                std::istreambuf_iterator<char>(fShaderFile),
                std::istreambuf_iterator<char>()
        );
    }
    const char *shader_text_char_ptr = m_ShaderText.c_str();
    m_Shader = glCreateShader(shader_type);
    glShaderSource(m_Shader, 1, &shader_text_char_ptr, nullptr);
    glCompileShader(m_Shader);

    glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &m_Compiled);
    if (!m_Compiled) {
        int infoLen = 0;
        glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char infoLog[2000];
            glGetShaderInfoLog(m_Shader, infoLen, nullptr, infoLog);
            std::cerr << "Error compiling " << filename << ": " << infoLog << "\n";
        }
        glDeleteShader(m_Shader);
        //return 0;
    }
}

ShaderProgram::ShaderProgram(const char* base_path) {
    Shader vertex_shader(GL_VERTEX_SHADER, (std::string(base_path) + ".vert").c_str());
    Shader fragment_shader(GL_FRAGMENT_SHADER, (std::string(base_path) + ".frag").c_str());
    handle = glCreateProgram();
    glAttachShader(handle, vertex_shader.getShaderId());
    glAttachShader(handle, fragment_shader.getShaderId());
    glLinkProgram(handle);
}

GLint ShaderProgram::uniformLocation(const char* name) const {
    return glGetUniformLocation(handle, name);
}

void ShaderProgram::activate() const {
    glUseProgram(handle);
}