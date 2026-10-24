#include <iostream>

#include "highmap.hpp"

int main(void)
{
  hmap::gpu::init_opencl(); // for rocks.to_heightmap method

  glm::ivec2 shape = {1024, 1024};
  glm::vec2  kw = {3.f, 3.f};
  int        seed = 0;

  const float slope_max = 4.f / shape.x;

  // --- Terrain

  hmap::Array z = hmap::noise_fbm(hmap::NoiseType::SIMPLEX2, shape, kw, seed);
  hmap::hydraulic_mise(z);

  // --- Rock density (using build_rock_density with internal bulkify)

  hmap::Array density = hmap::build_rock_density(z, slope_max, 0.01f);
  density.dump("density.png");

  // --- Procedural rock placement

  hmap::RockSeedingOptions options;
  options.seed = static_cast<uint32_t>(seed);
  options.distribution = hmap::RockDistribution(0, 0.0005f, 0.01f);

  hmap::RockField rocks = hmap::place_rocks_procedural(
      10000,
      z,
      z.min(),
      z.max(),
      slope_max,
      density,
      {},   // exclusion
      0.5f, // size_slope_weight
      1.f,  // size_elevation_gain
      1.5f, // size_elevation_exponent
      options);

  std::cout << rocks.to_string() << "\n";

  // --- Export

  hmap::Array rock_elevation = rocks.to_heightmap(shape,
                                                  hmap::SCATTER_SHAPE_POLYGON,
                                                  2.f);
  hmap::Array z_with_rocks = z + 10.f * rock_elevation;

  rocks.to_png("rock_procedural_density.png", shape, z);
  hmap::export_banner_png("ex_rock_procedural.png",
                          {z, density, z_with_rocks},
                          hmap::Cmap::TERRAIN,
                          true,
                          true);

  return 0;
}
