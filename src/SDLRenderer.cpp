#include "SDLRenderer.h"
#include <iostream>

SDLRenderer::~SDLRenderer() { cleanup(); }

bool SDLRenderer::init(int width, int height, const char *title) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return false;
  }

  win_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          width, height, SDL_WINDOW_SHOWN);
  if (!win_) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
    SDL_Quit();
    return false;
  }

  // Use hardware-accelerated renderer if available
  ren_ = SDL_CreateRenderer(
      win_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren_) {
    // fallback to software if needed
    ren_ = SDL_CreateRenderer(win_, -1, 0);
  }
  if (!ren_) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(win_);
    win_ = nullptr;
    SDL_Quit();
    return false;
  }

  // Create streaming texture in ARGB8888 format
  tex_ = SDL_CreateTexture(ren_, SDL_PIXELFORMAT_ARGB8888,
                           SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!tex_) {
    std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
    SDL_DestroyRenderer(ren_);
    ren_ = nullptr;
    SDL_DestroyWindow(win_);
    win_ = nullptr;
    SDL_Quit();
    return false;
  }

  width_ = width;
  height_ = height;
  return true;
}

void SDLRenderer::updateTexture(const uint32_t *pixels, int pitch) {
  if (!tex_)
    return;
  // pitch is provided in bytes-per-row by our callers
  if (SDL_UpdateTexture(tex_, nullptr, pixels, pitch) != 0) {
    std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << "\n";
  }
}

void SDLRenderer::present() {
  if (!ren_ || !tex_)
    return;
  SDL_RenderClear(ren_);
  SDL_RenderCopy(ren_, tex_, nullptr, nullptr);
  SDL_RenderPresent(ren_);
}

bool SDLRenderer::pollEvents(std::vector<Event> &outEvents) {
  outEvents.clear();
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
    case SDL_QUIT: {
      Event e;
      e.type = Event::Type::Quit;
      outEvents.push_back(e);
      // signal quit to caller
      return false;
    }
    case SDL_KEYDOWN: {
      Event e;
      e.type = Event::Type::KeyDown;
      e.code = static_cast<int>(ev.key.keysym.sym); // SDL_Keycode value as int
      outEvents.push_back(e);
      break;
    }
    case SDL_KEYUP: {
      Event e;
      e.type = Event::Type::KeyUp;
      e.code = static_cast<int>(ev.key.keysym.sym);
      outEvents.push_back(e);
      break;
    }
    case SDL_MOUSEMOTION: {
      Event e;
      e.type = Event::Type::MouseMove;
      e.x = ev.motion.x;
      e.y = ev.motion.y;
      outEvents.push_back(e);
      break;
    }
    // Add more SDL -> Event translations here if needed (mouse buttons, wheel,
    // etc.)
    default:
      break;
    }
  }

  // No quit event encountered: continue running
  return true;
}

void SDLRenderer::cleanup() {
  if (tex_) {
    SDL_DestroyTexture(tex_);
    tex_ = nullptr;
  }
  if (ren_) {
    SDL_DestroyRenderer(ren_);
    ren_ = nullptr;
  }
  if (win_) {
    SDL_DestroyWindow(win_);
    win_ = nullptr;
  }
  // It's okay to call SDL_Quit multiple times; ensure we only call it if we
  // initialized SDL.
  SDL_Quit();
}
