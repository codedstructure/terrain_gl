// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_TEXTURE_H
#define TERRAIN_GL_TEXTURE_H

class Texture {
public:
    Texture(const int tex_id, const char* filename);
    ~Texture();

    unsigned int get_id() {return m_texture_id;}
private:
    unsigned char* m_data;
    int m_width;
    int m_height;
    int m_channels;
    unsigned int m_texture_id;
};

#endif //TERRAIN_GL_TEXTURE_H
