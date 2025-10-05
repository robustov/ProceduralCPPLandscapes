
#include "SDLRenderer.h"
#include "Scene.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>

static inline MountainColorScheme getNordScheme() {
  MountainColorScheme s;
  s.skyTop = 0xFF2E3440u;
  s.skyBottom = 0xFFD8DEE9u;
  s.fogColor = lerpColor(s.skyBottom, 0xFF88C0D0u, 0.07);
  s.sunColor = 0xFF8FBCBBu;
  return s;
}

static inline MountainColorScheme getEverforestScheme() {
  MountainColorScheme s;
  s.skyTop = 0xFF1E2326u;
  s.skyBottom = 0xFFE67E80u;
  s.fogColor = 0x00D8DEE9u;
  s.sunColor = 0xFFF6E9B3u;
  return s;
}

static const std::vector<uint32_t> NORD_PALETTE = {
    0xFF2E3440u, 0xFF3B4252u, 0xFF434C5Eu, 0xFF4C566Au,
    0xFFD8DEE9u, 0xFF8FBCBBu, 0xFF88C0D0u, 0xFF81A1C1u};
static const std::vector<uint32_t> EVER_PALETTE = {
    0xFF1B3B2Bu, 0xFF2A5B3Au, 0xFF3B7B49u, 0xFF4C9B58u, 0xFF6BBA6Bu};

std::vector<MountainParams>
makeRandomMountainsWithPalette(size_t count, int winW,
                               const std::vector<uint32_t> &palette,
                               const MountainColorScheme &scheme) {
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
    p.roughness = 0.48;
    p.minHeight = 0;
    p.maxHeight = 1.5 - t;
    p.verticalSpan = 0.5 + 0.45 * t;
    p.verticalOffset = static_cast<int>(30.0 * (1.0 + t));

    uint32_t baseColor;
    if (!palette.empty()) {
      size_t idx = std::min<size_t>(i, palette.size() - 1);
      baseColor = palette[idx];
    } else {
      uint8_t g = uint8_t(30 + (rng() % 140));
      baseColor = packARGB(0xFF, g, g, g);
    }

    double fogStrength = 0 * (1.0 - t); // tweak multiplier to taste
    p.colorARGB = lerpColor(baseColor, scheme.fogColor, fogStrength);

    out.push_back(std::move(p));
  }
  return out;
}

std::vector<MountainParams>
makeRandomMountainsNord(size_t count, int winW,
                        const MountainColorScheme &scheme) {
  return makeRandomMountainsWithPalette(count, winW, NORD_PALETTE, scheme);
}
std::vector<MountainParams>
makeRandomMountainsEverforest(size_t count, int winW,
                              const MountainColorScheme &scheme) {
  return makeRandomMountainsWithPalette(count, winW, EVER_PALETTE, scheme);
}

int main() {
  const int WIN_W = 1024;
  const int WIN_H = 512;

  SDLRenderer renderer;
  if (!renderer.init(WIN_W, WIN_H, "Mountains"))
    return 1;

  MountainColorScheme currentScheme = getNordScheme();
  std::vector<uint32_t> currentPalette = NORD_PALETTE;
  size_t currentCount = 3;
  Scene scene =
      Scene::makeDefault(WIN_W, WIN_H, static_cast<int>(currentCount));

  Scene newScene(WIN_W, WIN_H, currentScheme);
  auto params = makeRandomMountainsWithPalette(currentCount, WIN_W,
                                               currentPalette, currentScheme);
  for (auto &p : params)
    newScene.addMountain(std::move(p));
  scene = std::move(newScene);

  std::vector<uint32_t> buffer(WIN_W * WIN_H);
  const int rowStride = WIN_W;

  bool running = true;
  std::vector<Event> events;
  auto last = std::chrono::steady_clock::now();

  while (running) {

    running = renderer.pollEvents(events);

    bool regenRequested = false;
    bool switchPaletteNord = false;
    bool switchPaletteEver = false;
    bool switchPaletteRandom = false;
    int numericKeyPressed = -1; // -1 none, otherwise 1..10

    for (const auto &e : events) {
      if (e.type == Event::Type::Quit) {
        running = false;
        break;
      }
      if (e.type == Event::Type::KeyDown) {
        int kc = e.code;
        if (kc == SDLK_SPACE)
          regenRequested = true;
        else if (kc == SDLK_n)
          switchPaletteNord = true;
        else if (kc == SDLK_e)
          switchPaletteEver = true;
        else if (kc == SDLK_r)
          switchPaletteRandom = true;
        else if (kc >= SDLK_0 && kc <= SDLK_9) {
          numericKeyPressed = (kc == SDLK_0) ? 10 : (kc - SDLK_0);
        } else if (kc == SDLK_q || kc == SDLK_ESCAPE) {
          running = false;
          break;
        }
      }
    }

    if (!running)
      break;

    if (switchPaletteNord) {
      currentScheme = getNordScheme();
      currentPalette = NORD_PALETTE;
      scene.setScheme(currentScheme);
    } else if (switchPaletteEver) {
      currentScheme = getEverforestScheme();
      currentPalette = EVER_PALETTE;
      scene.setScheme(currentScheme);
    } else if (switchPaletteRandom) {
      currentPalette.clear();
      currentScheme = {};
      scene.setScheme(currentScheme);
    }

    if (numericKeyPressed > 0) {
      size_t n = static_cast<size_t>(numericKeyPressed);

      Scene newScene(WIN_W, WIN_H, currentScheme);
      auto params = makeRandomMountainsWithPalette(n, WIN_W, currentPalette,
                                                   currentScheme);
      for (auto &p : params)
        newScene.addMountain(std::move(p));
      scene = std::move(newScene);
      currentCount = n;
    } else if (regenRequested) {

      Scene newScene(WIN_W, WIN_H, currentScheme);
      auto params = makeRandomMountainsWithPalette(
          currentCount, WIN_W, currentPalette, currentScheme);
      for (auto &p : params)
        newScene.addMountain(std::move(p));
      scene = std::move(newScene);
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> dt = now - last;
    last = now;

    scene.update(dt.count());
    scene.render(buffer.data(), rowStride);

    renderer.updateTexture(buffer.data(), rowStride * 4);
    renderer.present();

    SDL_Delay(8);
  }

  renderer.cleanup();
  return 0;
}
