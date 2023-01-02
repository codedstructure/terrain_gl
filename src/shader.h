// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_SHADER_H
#define TERRAIN_GL_SHADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>

using std::basic_ifstream;
using std::basic_string;

class Shader
{
    public:
        Shader(unsigned int shader_type, const char* filename);
        [[nodiscard]] unsigned int getShaderId() const { return m_Shader; }
    private:
        unsigned int m_Shader;
        int m_Compiled;

        basic_string<char> m_ShaderText;
};

class ShaderProgram
{
public:
    explicit ShaderProgram(const char* base_path);

    void activate() const;
    GLint uniformLocation(const char* name) const;
private:
    int handle;
};

#endif //TERRAIN_GL_SHADER_H
