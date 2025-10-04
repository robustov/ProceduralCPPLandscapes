// include/IRenderer.h
#pragma once
#include <cstdint>
#include <vector>

struct Event {
  enum class Type : uint8_t {
    Quit,
    KeyDown,
    KeyUp,
    MouseMove, // extend later if needed
    // ... add more event kinds as needed
  } type;

  // For keyboard events, this will be the SDL_Keycode equivalent (or app-chosen
  // key code). We use int so backends can pass platform codes. Apps should map
  // codes to actions.
  int code = 0;

  // Mouse fields (unused for now)
  int x = 0, y = 0;
};

struct IRenderer {
  virtual ~IRenderer() = default;
  virtual bool init(int width, int height, const char *title) = 0;
  // updateTexture expects pixels in ARGB8888 and pitch in bytes
  virtual void updateTexture(const uint32_t *pixels, int pitch) = 0;
  virtual void present() = 0;

  // Poll platform events and append them to 'outEvents'.
  // Returns true to continue running, false to quit (e.g. Quit event).
  virtual bool pollEvents(std::vector<Event> &outEvents) = 0;

  virtual void cleanup() = 0;
};
