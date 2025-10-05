#pragma once
#include "Mountain.h"
#include "MountainLayer.h"
#include "RenderUtils.h"
#include <MountainColorScheme.h>
#include <memory>
#include <vector>

class Scene {
public:
  Scene(int width, int height, const MountainColorScheme &scheme);
  static Scene makeDefault(int width, int height, int mountainCount = 3);
  void addMountain(MountainParams params);
  void addLayer(std::unique_ptr<Layer> layer);
  void clearLayers();
  void update(double dt);
  void render(uint32_t *pixels, int rowStride);
  const MountainColorScheme &scheme() const { return scheme_; }
  void setScheme(const MountainColorScheme &s) { scheme_ = s; }
  void setMountains(std::vector<MountainParams> paramsList);
  void clearMountains(); // convenience
  std::vector<Mountain> &getMountains() { return mountains_; }
  const std::vector<Mountain> &getMountains() const { return mountains_; }

private:
  int width_, height_;
  MountainColorScheme scheme_;
  std::vector<Mountain> mountains_;
  std::vector<std::unique_ptr<Layer>> layers_;
};
