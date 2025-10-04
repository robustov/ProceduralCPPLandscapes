#include "SDLRenderer.h"
#include "Scene.h"
#include <ctime>
#include <iostream>
#include <vector>

std::vector<MountainParams> makeRandomMountains(size_t n, int winW) {
  std::vector<MountainParams> out;
  out.reserve(n);
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  for (size_t i = 0; i < n; ++i) {
    MountainParams p;
    p.width = winW;
    p.seed = static_cast<uint32_t>(std::rand()) ^
             static_cast<uint32_t>(i * 2654435761u);
    p.leftHeight = 0.05 + 0.15 * ((double)std::rand() / RAND_MAX);
    p.rightHeight = 0.05 + 0.15 * ((double)std::rand() / RAND_MAX);
    p.initialDisplacement = 0.6 + 0.6 * ((double)std::rand() / RAND_MAX);
    p.roughness = 0.4 + 0.2 * ((double)std::rand() / RAND_MAX);
    p.minHeight = 0.02;
    p.maxHeight = 0.5;
    p.verticalSpan = 0.55 + 0.35 * ((double)std::rand() / RAND_MAX);
    p.verticalOffset = 30 - int(i * 5);
    uint8_t g = uint8_t(30 + std::rand() % 120);
    p.colorARGB =
        (0xFFu << 24) | (uint32_t(g) << 16) | (uint32_t(g) << 8) | uint32_t(g);
    out.push_back(std::move(p));
  }
  return out;
}

enum class PaletteId { RandomGray, Nord, Everforest };
PaletteId currentPaletteId = PaletteId::RandomGray;
std::vector<uint32_t> currentPalette; // empty = random gray fallback

std::vector<MountainParams>
makeRandomMountainsWithPalette(size_t count, int winW,
                               const std::vector<uint32_t> &palette) {
  std::vector<MountainParams> out;
  out.reserve(count);
  static thread_local std::mt19937 rng((uint32_t)std::random_device{}());
  std::uniform_real_distribution<double> uf01(0.0, 1.0);
  for (size_t i = 0; i < count; ++i) {
    double t = double(i) / double(std::max<size_t>(1, count - 1));
    MountainParams p;
    p.width = winW;
    p.seed = static_cast<uint32_t>(rng() ^ (uint32_t)(i * 2654435761u));
    p.leftHeight = 0.05 + 0.15 * uf01(rng) + 0.06 * t;
    p.rightHeight = 0.05 + 0.15 * uf01(rng) + 0.06 * t;
    p.initialDisplacement = 0.5 + 0.9 * uf01(rng) + 0.4 * t;
    p.roughness = 0.45 + 0.18 * uf01(rng);
    p.minHeight = 0.02 * (1.0 - t);
    p.maxHeight = 0.35 + 0.55 * t; // foreground larger
    p.verticalSpan = 0.5 + 0.45 * t;
    p.verticalOffset = static_cast<int>(30.0 * (1.0 - t));
    if (!palette.empty()) {
      size_t idx = std::min<size_t>(i, palette.size() - 1);
      p.colorARGB = palette[idx];
    } else {
      uint8_t g = uint8_t(30 + (rng() % 140));
      p.colorARGB = (0xFFu << 24) | (uint32_t(g) << 16) | (uint32_t(g) << 8) |
                    uint32_t(g);
    }
    out.push_back(std::move(p));
  }
  return out;
}

// Nord palette wrapper
std::vector<MountainParams> makeRandomMountainsNord(size_t count, int winW) {
  // Nord-ish palette (far -> near)
  const std::vector<uint32_t> nord = {0xFF2E3440u, // polar-night
                                      0xFF3B4252u, 0xFF434C5Eu, 0xFF4C566Au,
                                      0xFFD8DEE9u, // snow-light (use sparingly)
                                      0xFF8FBCBBu, // frost / teal
                                      0xFF88C0D0u, 0xFF81A1C1u};
  return makeRandomMountainsWithPalette(count, winW, nord);
}

// Everforest palette wrapper
std::vector<MountainParams> makeRandomMountainsEverforest(size_t count,
                                                          int winW) {
  const std::vector<uint32_t> everforest = {
      0xFF1B3B2Bu, // far (deep green)
      0xFF2A5B3Au, 0xFF3B7B49u, 0xFF4C9B58u,
      0xFF6BBA6Bu // near
  };
  return makeRandomMountainsWithPalette(count, winW, everforest);
}

int main() {

  // helper to set the palette vector from id
  auto setPaletteById = [&](PaletteId id) {
    currentPalette.clear();
    currentPaletteId = id;
    if (id == PaletteId::Nord) {
      currentPalette = {0xFF2E3440u, 0xFF3B4252u, 0xFF434C5Eu, 0xFF4C566Au,
                        0xFFD8DEE9u, 0xFF8FBCBBu, 0xFF88C0D0u, 0xFF81A1C1u};
    } else if (id == PaletteId::Everforest) {
      currentPalette = {0xFF1B3B2Bu, 0xFF2A5B3Au, 0xFF3B7B49u, 0xFF4C9B58u,
                        0xFF6BBA6Bu};
    } else {
      // Random gray: empty palette => function will produce grayscale colors
      currentPalette.clear();
    }
  };

  setPaletteById(PaletteId::RandomGray);
  const int WIN_W = 1024, WIN_H = 512;
  SDLRenderer renderer;
  if (!renderer.init(WIN_W, WIN_H, "Mountains lib demo"))
    return 1;

  Scene scene = Scene::makeDefault(WIN_W, WIN_H, 3);
  scene.setMountains(makeRandomMountains(3, WIN_W));
  std::vector<uint32_t> buffer(WIN_W * WIN_H);
  const int rowStride = WIN_W;

  bool running = true;
  std::vector<Event> events;

  while (running) {
    running = renderer.pollEvents(events);

    // handle events at app level
    bool regenRequested = false;
    bool everforestRequested = false;
    for (auto &e : events) {
      if (e.type == Event::Type::Quit) {
        running = false;
        break;
      }
      if (e.type == Event::Type::KeyDown) {
        int kc = e.code;
        if (kc == SDLK_n) {
          setPaletteById(PaletteId::Nord);
        } else if (kc == SDLK_e) {
          setPaletteById(PaletteId::Everforest);
        } else if (kc == SDLK_r) {
          setPaletteById(PaletteId::RandomGray);
        } else if (kc >= SDLK_0 && kc <= SDLK_9) {
          int n = (kc == SDLK_0) ? 10 : (kc - SDLK_0);
          scene.setMountains(
              makeRandomMountainsWithPalette((size_t)n, WIN_W, currentPalette));
        } else if (kc == SDLK_SPACE) {
          size_t count = scene.getMountains().size();
          scene.setMountains(
              makeRandomMountainsWithPalette(count, WIN_W, currentPalette));
        }
      }
    }

    scene.update(100.0 / 60.0);
    scene.render(buffer.data(), rowStride);
    renderer.updateTexture(buffer.data(), rowStride * 4);
    renderer.present();
    SDL_Delay(1);
  }

  renderer.cleanup();
  return 0;
}
