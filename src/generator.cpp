#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using u32 = uint32_t;

void midpoint_displace(std::vector<double> &h, int left, int right, double disp,
                       std::mt19937 &rng) {
  if (right - left <= 1)
    return;
  int mid = (left + right) / 2;
  std::uniform_real_distribution<double> d(-disp, disp);
  double midv = 0.5 * (h[left] + h[right]) + d(rng);
  h[mid] = midv;
  double nextDisp = disp * 0.4;
  midpoint_displace(h, left, mid, nextDisp, rng);
  midpoint_displace(h, mid, right, nextDisp, rng);
}

std::vector<double> generate_heightline(int targetWidth, double leftH,
                                        double rightH, double initialDisp,
                                        uint32_t seed) {
  if (targetWidth < 3)
    targetWidth = 3;
  int k = 1;
  while (((1 << k) + 1) < targetWidth)
    ++k;
  int n = (1 << k) + 1;
  std::vector<double> h(n, 0.0);
  h.front() = leftH;
  h.back() = rightH;
  std::mt19937 rng(seed);
  midpoint_displace(h, 0, n - 1, initialDisp, rng);

  double mn = *std::min_element(h.begin(), h.end());
  double mx = *std::max_element(h.begin(), h.end());
  double rngv = mx - mn;
  if (rngv <= 0.0)
    rngv = 1.0;
  for (double &v : h)
    v = (v - mn) / rngv;
  std::vector<double> out(targetWidth);
  for (int i = 0; i < targetWidth; ++i)
    out[i] = h[i];
  return out;
}

int main(int argc, char **argv) {
  const int WIN_W = 1024;
  const int WIN_H = 512;
  const char *title = "Midpoint Mountains (SDL2)";

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
    return 1;
  }

  SDL_Window *win =
      SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WIN_W, WIN_H, SDL_WINDOW_SHOWN);
  if (!win) {
    std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  if (!ren) {
    std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  SDL_Texture *tex = SDL_CreateTexture(
      ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H);
  if (!tex) {
    std::cerr << "SDL_CreateTexture error: " << SDL_GetError() << "\n";
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 1;
  }

  const u32 SKY_COLOR = 0xFF87C2FF; // light blue-ish sky
  const u32 MOUNTAIN_COLOR = 0xFF2E2E2E;

  double leftH = 0.25;
  double rightH = 0.25;
  double initialDisp = 0.8;

  uint32_t seed = 42;
  std::mt19937 rng(seed);

  auto heights = generate_heightline(WIN_W, leftH, rightH, initialDisp, seed);

  bool running = true;
  SDL_Event ev;
  const int targetFPS = 60;
  const int frameDelay = 1000 / targetFPS;

  while (running) {
    uint32_t frameStart = SDL_GetTicks();
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT)
        running = false;
      else if (ev.type == SDL_KEYDOWN) {
        SDL_Keycode k = ev.key.keysym.sym;
        if (k == SDLK_ESCAPE || k == SDLK_q)
          running = false;
        else if (k == SDLK_SPACE) {
          seed = rng();
          heights =
              generate_heightline(WIN_W, leftH, rightH, initialDisp, seed);
          std::cout << "Regenerated seed=" << seed << "\n";
        }
      }
    }

    void *pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(tex, nullptr, &pixels, &pitch) != 0) {
      std::cerr << "SDL_LockTexture: " << SDL_GetError() << "\n";
      break;
    }
    u32 *px = static_cast<u32 *>(pixels);
    int rowStride = pitch / 4;

    for (int y = 0; y < WIN_H; ++y) {
      for (int x = 0; x < WIN_W; ++x) {
        px[y * rowStride + x] = SKY_COLOR;
      }
    }

    for (int x = 0; x < WIN_W; ++x) {
      double h = heights[x];
      int topY = static_cast<int>((1.0 - h) * (WIN_H * 0.75));
      if (topY < 0)
        topY = 0;
      if (topY >= WIN_H)
        topY = WIN_H - 1;
      for (int y = topY; y < WIN_H; ++y) {
        px[y * rowStride + x] = MOUNTAIN_COLOR;
      }
    }

    for (int x = 0; x < WIN_W; ++x) {
      double h = heights[x]; // normalized
      int topY = static_cast<int>((1.0 - h) * (WIN_H * 0.75));
      if (topY < 0)
        topY = 0;
      if (topY >= WIN_H)
        topY = WIN_H - 1;
      for (int y = topY; y < WIN_H; ++y) {
        px[y * rowStride + x] = MOUNTAIN_COLOR;
      }
    }
    SDL_UnlockTexture(tex);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);

    uint32_t frameTime = SDL_GetTicks() - frameStart;
    if (frameDelay > frameTime)
      SDL_Delay(frameDelay - frameTime);
  }

  SDL_DestroyTexture(tex);
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
