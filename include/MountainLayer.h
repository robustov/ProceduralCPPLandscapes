#pragma once
#include "Layer.h"
#include "Mountain.h"
#include <vector>

class MountainLayer : public Layer {
public:
  MountainLayer(std::vector<Mountain> mountains);
  void update(double dt) override;
  void render(const RenderContext &ctx) override;
  std::vector<Mountain> &mountains() { return mountains_; }

private:
  std::vector<Mountain> mountains_;
};
