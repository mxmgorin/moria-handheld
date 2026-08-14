// What can this device's SDL2 actually do? Not shipped.
//
// For every render driver in turn: paint the screen, then paint it again by way
// of a render target. Each stage says what it did and what SDL thought of it,
// and the screen shows the answer even if the log never makes it off the card:
//
//   red    the driver can put pixels on the panel
//   green  it can do that through a render target, which is how the game draws
//
// Build: zig cc -target arm-linux-gnueabihf.2.28 -I<sdl headers> -o probe \
//          tools/dev/probe.c -lSDL2 -Wl,--allow-shlib-undefined
#include <SDL.h>

enum { W = 640, H = 480, HOLD_MS = 2500 };

static void
paint(SDL_Renderer* renderer, int r, int g, int b)
{
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  SDL_Delay(HOLD_MS);
}

int
main(int argc, char** argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("SDL_Init: %s", SDL_GetError());
    return 1;
  }
  SDL_Log("video driver: %s", SDL_GetCurrentVideoDriver());

  SDL_Window* window = SDL_CreateWindow("probe", 0, 0, W, H, 0);
  if (!window) {
    SDL_Log("SDL_CreateWindow: %s", SDL_GetError());
    return 1;
  }
  int ww = 0, wh = 0;
  SDL_GetWindowSize(window, &ww, &wh);
  SDL_Log("window %dx%d", ww, wh);

  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_Log("window surface: %s", surface ? "yes" : SDL_GetError());

  int count = SDL_GetNumRenderDrivers();
  for (int it = 0; it < count; ++it) {
    SDL_RendererInfo info;
    SDL_GetRenderDriverInfo(it, &info);
    SDL_Log("--- driver %d: %s (flags 0x%x, max texture %dx%d)", it, info.name,
            info.flags, info.max_texture_width, info.max_texture_height);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, it, 0);
    if (!renderer) {
      SDL_Log("  CreateRenderer: %s", SDL_GetError());
      continue;
    }

    // The recipe known to work on this panel: a streaming ABGR8888 texture,
    // filled on the CPU, blend off, presented as one copy. Anything else --
    // clears, primitives, other formats -- mmiyoo drops without erroring.
    SDL_Texture* stream =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                          SDL_TEXTUREACCESS_STREAMING, W, H);
    SDL_Log("  streaming ABGR8888 texture: %s",
            stream ? "created" : SDL_GetError());
    if (stream) {
      SDL_SetTextureBlendMode(stream, SDL_BLENDMODE_NONE);
      void* pixels = 0;
      int pitch = 0;
      if (SDL_LockTexture(stream, 0, &pixels, &pitch) != 0) {
        SDL_Log("  LockTexture: %s", SDL_GetError());
      } else {
        for (int y = 0; y < H; ++y) {
          uint32_t* row = (uint32_t*)((char*)pixels + y * pitch);
          for (int x = 0; x < W; ++x) row[x] = 0xff2020c8;  // red, ABGR
        }
        SDL_UnlockTexture(stream);
        SDL_Log("  copying the streaming texture (expect red)");
        SDL_RenderCopy(renderer, stream, 0, 0);
        SDL_RenderPresent(renderer);
        SDL_Delay(HOLD_MS);
      }
      SDL_DestroyTexture(stream);
    }

    SDL_Log("  RenderTargetSupported %d", SDL_RenderTargetSupported(renderer));
    SDL_Texture* target =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                          SDL_TEXTUREACCESS_TARGET, W, H);
    SDL_Log("  ABGR8888 target texture: %s", target ? "created" : SDL_GetError());

    if (target) {
      SDL_SetTextureBlendMode(target, SDL_BLENDMODE_NONE);
      if (SDL_SetRenderTarget(renderer, target) != 0) {
        SDL_Log("  SetRenderTarget: %s", SDL_GetError());
      } else {
        SDL_SetRenderDrawColor(renderer, 32, 200, 32, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, 0);
        SDL_Log("  copying the target (expect green)");
        SDL_RenderCopy(renderer, target, 0, 0);
        SDL_RenderPresent(renderer);
        SDL_Delay(HOLD_MS);
        SDL_Log("  target path survived");
      }
      SDL_DestroyTexture(target);
    }

    SDL_DestroyRenderer(renderer);
  }

  SDL_Log("probe done");
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
