#include "Scene.h"
#include "MountainParams.h"

#include <algorithm>
Scene::Scene(int width, int height, const MountainColorScheme &scheme)
    : width_(width), height_(height), scheme_(scheme) {}

Scene Scene::makeDefault(int width, int height, int mountainCount) {
  MountainColorScheme cs;
  Scene s(width, height, cs);
  for (int i = 0; i < mountainCount; ++i) {
    MountainParams p;
    p.width = width;
    p.seed = 1000u + (i + 1) * 111;
    double t = double(i) / double(std::max(1, mountainCount - 1));
    p.leftHeight = 0.1 + 0.2 * t;
    p.rightHeight = 0.1 + 0.15 * t;
    p.initialDisplacement = 0.8 + 0.3 * t;
    p.minHeight = 0.1 * (1.0 - t);
    p.maxHeight = 0.5 + 0.4 * t;
    p.verticalSpan = 0.6 + 0.25 * (1.0 - t);
    uint8_t gray = uint8_t(40 + 40 * (1.0 - t));
    p.colorARGB = (0xFFu << 24) | (uint32_t(gray) << 16) |
                  (uint32_t(gray) << 8) | uint32_t(gray);
    s.addMountain(std::move(p));
  }
  return s;
}

void Scene::addMountain(MountainParams params) {
  mountains_.emplace_back(std::move(params));
}

void Scene::addLayer(std::unique_ptr<Layer> layer) {
  layers_.push_back(std::move(layer));
}

void Scene::clearLayers() { layers_.clear(); }

void Scene::update(double dt) {
  for (auto &l : layers_)
    l->update(dt);
}

void Scene::render(uint32_t *pixels, int rowStride) {
  fill_sky_gradient(pixels, rowStride, width_, height_, scheme_.skyTop,
                    scheme_.skyBottom);
  RenderContext ctx{pixels, rowStride, width_, height_};
  for (auto &l : layers_)
    l->render(ctx);
  for (const auto &m : mountains_)
    m.paint(pixels, rowStride, width_, height_);
}

void Scene::clearMountains() { mountains_.clear(); }

void Scene::setMountains(std::vector<MountainParams> paramsList) {
  mountains_.clear();
  mountains_.reserve(paramsList.size());
  for (auto &p : paramsList) {
    mountains_.emplace_back(std::move(p));
  }
}
