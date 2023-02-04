// terrain_gl
// @codedstructure 2023

#ifndef TERRAIN_GL_HEIGHTMAP_H
#define TERRAIN_GL_HEIGHTMAP_H

#include <map>
#include <vector>

template<typename T>
class HeightMap {
public:
  HeightMap(int grid_size, int grid_scale);

  std::vector<T>& getPatch(int x, int y);
  std::vector<GLfloat> grid;
  std::vector<GLuint> grid_indices;

  int size;
  // size of grid in world units
  int grid_scale;
private:
  void generatePatch(int x, int y, std::vector<T>& target);
  std::map<std::pair<int, int>, std::vector<T>> patches;
};

#endif //TERRAIN_GL_HEIGHTMAP_H
