#pragma once
#include "Mountain.h"
#include "MountainLayer.h"
#include "RenderUtils.h"
#include <memory>
#include <vector>

struct ColorScheme {
  uint32_t skyTop = 0xFF2878DC;
  uint32_t skyBottom = 0xFFE8F4FF;
  uint32_t mountainColor = 0xFF2E2E2E;
  uint32_t cloudColor = 0xFFFFFFFF;
};

class Scene {
public:
  Scene(int width, int height, const ColorScheme &scheme);
  static Scene makeDefault(int width, int height, int mountainCount = 3);
  void addMountain(MountainParams params);
  void addLayer(std::unique_ptr<Layer> layer);
  void clearLayers();
  void update(double dt);
  void render(uint32_t *pixels, int rowStride);
  const ColorScheme &scheme() const { return scheme_; }
  void setScheme(const ColorScheme &s) { scheme_ = s; }
  void setMountains(std::vector<MountainParams> paramsList);
  void clearMountains(); // convenience
  std::vector<Mountain> &getMountains() { return mountains_; }
  const std::vector<Mountain> &getMountains() const { return mountains_; }

private:
  int width_, height_;
  ColorScheme scheme_;
  std::vector<Mountain> mountains_;
  std::vector<std::unique_ptr<Layer>> layers_;
};
