#include "Color.h"
#include "Mountain.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <random>
#include <vector>

#include <random>
struct Cloud {
  float cx, cy; // center (pixels)
  float rx, ry; // radii
  double alpha; // max alpha 0..1
};

double gaussian_falloff(double dx, double dy, double rx, double ry) {
  double nx = dx / rx;
  double ny = dy / ry;
  double r2 = nx * nx + ny * ny;
  // use exp(-r2 * k) shape; tweak k for softness
  return std::exp(-r2 * 2.0);
}

void draw_clouds(uint32_t *pixels, int rowStride, int winW, int winH,
                 const std::vector<Cloud> &clouds, uint32_t cloudColorARGB) {
  for (const auto &c : clouds) {
    int x0 = std::max(0, int(std::floor(c.cx - c.rx)));
    int x1 = std::min(winW - 1, int(std::ceil(c.cx + c.rx)));
    int y0 = std::max(0, int(std::floor(c.cy - c.ry)));
    int y1 = std::min(winH - 1, int(std::ceil(c.cy + c.ry)));
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        double dx = double(x) - c.cx;
        double dy = double(y) - c.cy;
        double f = gaussian_falloff(dx, dy, c.rx, c.ry);
        if (f < 1e-3)
          continue;
        double a = c.alpha * f; // final alpha [0..1]

        // blend cloudColor over dest
        uint32_t dst = pixels[y * rowStride + x];
        // unpack
        int dra = (dst >> 24) & 0xFF, dr = (dst >> 16) & 0xFF,
            dg = (dst >> 8) & 0xFF, db = dst & 0xFF;
        int cra = (cloudColorARGB >> 24) & 0xFF,
            cr = (cloudColorARGB >> 16) & 0xFF,
            cg = (cloudColorARGB >> 8) & 0xFF, cb = cloudColorARGB & 0xFF;
        double alpha = a; // 0..1
        // simple alpha blend (source alpha over dest)
        double out_r = cr * alpha + dr * (1.0 - alpha);
        double out_g = cg * alpha + dg * (1.0 - alpha);
        double out_b = cb * alpha + db * (1.0 - alpha);
        // keep 255 alpha
        pixels[y * rowStride + x] =
            argb(255, uint8_t(out_r + 0.5), uint8_t(out_g + 0.5),
                 uint8_t(out_b + 0.5));
      }
    }
  }
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  const int WIN_W = 1024;
  const int WIN_H = 512;
  const char *title = "Mountains - refactor example";
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
  // Example: create several mountains with different styles/colors and
  // parameters
  std::vector<Mountain> mountains;
  // Back (further) mountain: wide, low-contrast, bluish
  mountains.emplace_back(
      WIN_W,     // width
      1001u,     // seed
      0.2, 0.15, // leftH, rightH
      0.8, 0.5,  // initialDisp, roughness (roughness isn't exposed in recursion
                 // multiplier here; tweak initialDisp)
      0.2, 0.6,  // minHeight, maxHeight (so this mountain stays lower)
      0.6,       // verticalSpan -> uses 60% of window height
      30,        // verticalOffset (push up by 30 px)
      Color{80, 100, 150, 255});
  // Middle mountain
  mountains.emplace_back(WIN_W, 2002u, 0.3, 0.3, 1.1, 0.5, 0.1, 0.9, 0.85, 0,
                         Color{50, 50, 70, 255});
  // Foreground mountain: darker, taller, narrower vertical span so it can be
  // half visible
  mountains.emplace_back(
      WIN_W, 3003u, 0.0, 0.0, 1.2, 0.5, 0.4, 0.95,
      0.5, // verticalSpan = 50% of window height (so mountain occupies only
           // half the height -> half hidden)
      0, Color{30, 30, 30, 255});
  bool running = true;
  SDL_Event ev;
  std::mt19937 seedGen(1234567u);
  const uint32_t SKY_COLOR = Color{150, 200, 255, 255}.toPixelARGB();
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
          // regenerate all mountains with fresh random seeds
          for (auto &m : mountains) {
            uint32_t newSeed = seedGen();
            m.regenerate(newSeed);
            std::cout << "Regenerated mountain with seed " << newSeed << "\n";
          }
        }
      }
    }
    // lock texture and fill pixels
    void *pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(tex, nullptr, &pixels, &pitch) != 0) {
      std::cerr << "SDL_LockTexture: " << SDL_GetError() << "\n";
      break;
    }
    uint32_t *px = static_cast<uint32_t *>(pixels);
    int rowStride = pitch / 4;
    // Fill with sky
    for (int y = 0; y < WIN_H; ++y) {
      for (int x = 0; x < WIN_W; ++x) {
        px[y * rowStride + x] = SKY_COLOR;
      }
    }
    // Paint mountains back-to-front
    for (const auto &m : mountains) {
      m.paint(px, rowStride, WIN_W, WIN_H);
    }

    std::vector<Cloud> clouds;
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> ux(0.0f, WIN_W),
        uy(0.0f, WIN_H * 0.4f);
    std::uniform_real_distribution<float> urx(60.0f, 180.0f), ury(20.0f, 70.0f),
        ua(0.12, 0.22);
    for (int i = 0; i < 6; ++i) {
      clouds.push_back({ux(rng), uy(rng), urx(rng), ury(rng), ua(rng)});
    }
    uint32_t cloudColor = argb(255, 255, 255, 255); // white clouds
    draw_clouds(px, rowStride, WIN_W, WIN_H, clouds, cloudColor);

    SDL_UnlockTexture(tex);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);
    // frame cap
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
