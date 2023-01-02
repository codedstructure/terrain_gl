// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_HEIGHTMAP_H
#define TERRAIN_GL_HEIGHTMAP_H

template<typename T>
class HeightMap {
public:
  HeightMap(int grid_size, int grid_scale);

  void generate();
  std::vector<GLfloat> grid;
  std::vector<GLuint> grid_indices;
  std::vector<T> heights;

  int size;
  // size of grid in world units
  int grid_scale;
};

#endif //TERRAIN_GL_HEIGHTMAP_H
