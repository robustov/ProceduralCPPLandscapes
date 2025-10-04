#pragma once
#include "IRenderer.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

class SDLRenderer : public IRenderer {
public:
  SDLRenderer() = default;
  ~SDLRenderer() override;

  // IRenderer interface
  bool init(int width, int height, const char *title) override;
  void updateTexture(const uint32_t *pixels,
                     int pitch) override; // pitch in bytes
  void present() override;

  // Poll platform events and append them to outEvents.
  // Returns true to continue running, false to quit (e.g. user closed window).
  bool pollEvents(std::vector<Event> &outEvents) override;

  void cleanup() override;

private:
  SDL_Window *win_ = nullptr;
  SDL_Renderer *ren_ = nullptr;
  SDL_Texture *tex_ = nullptr;
  int width_ = 0;
  int height_ = 0;
};
