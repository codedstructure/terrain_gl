//
// Created by Benjamin Bass on 16/02/2023.
//

#include <iostream>
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"

// Adapted from https://learnopengl.com/getting-started/textures
Texture::Texture(const char* filename) :
    m_data(nullptr),
    m_texture_id(0),
    m_width(0),
    m_height(0),
    m_channels(0)
{
    m_data = stbi_load(filename, &m_width, &m_height, &m_channels, 0); //// STBI_rgb_alpha);
    if (m_data == nullptr) {
        std::cerr << "Could not load texture from " << filename << "\n";
        return;
    }
    std::cout << "Loaded texture from " << filename << " " << m_width << "x" << m_height << "," << m_channels << "\n";
    glGenTextures(1, &m_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto format = m_channels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, m_data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() {
    if (m_data) {
        stbi_image_free(m_data);
    }
}