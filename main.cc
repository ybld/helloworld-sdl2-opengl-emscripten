// Copyright 2015 Red Blob Games <redblobgames@gmail.com>
// License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>

#include <iostream>
#include <SDL2/SDL.h>

#include "render.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static void SDLFAIL(const char* name) {
  std::cerr << name << " failed : " << SDL_GetError() << std::endl;
  exit(EXIT_FAILURE);
}

std::unique_ptr<Renderer> renderer;
SDL_Window* window;
static bool main_loop_running = true;
static bool main_loop_rendering = true;

float camera[2] = {0.0, 0.0};
float rotation = 0;

void main_loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      main_loop_running = false;
      break;
    }
    case SDL_KEYUP: {
      int sym = event.key.keysym.sym;
      if (sym == SDLK_ESCAPE) { main_loop_running = false; }
      break;
    }
    case SDL_MOUSEWHEEL: {
      // Panning is useful only if we have two axes from a trackpad; a
      // traditional scrollwheel should map to zoom. However we don't
      // know which is which.
      camera[0] += event.wheel.x * 0.1;
      camera[1] += event.wheel.y * 0.1;
      break;
    }
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_SHOWN: {
        main_loop_rendering = true;
        break;
      }
      case SDL_WINDOWEVENT_HIDDEN: {
        main_loop_rendering = false;
        break;
      }
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        renderer->HandleResize();
        break;
      }
      }
      break;
    }
    }
  }

  const Uint8* pressed = SDL_GetKeyboardState(nullptr);
  if (pressed[SDL_SCANCODE_Q]) { camera[0] -= 0.01; }
  if (pressed[SDL_SCANCODE_E]) { camera[0] += 0.01; }
  if (pressed[SDL_SCANCODE_W]) { camera[1] -= 0.01; }
  if (pressed[SDL_SCANCODE_S]) { camera[1] += 0.01; }
  if (pressed[SDL_SCANCODE_A]) { rotation -= 0.05; }
  if (pressed[SDL_SCANCODE_D]) { rotation += 0.05; }
  
  if (main_loop_rendering) {
    renderer->Render();
  }
}


int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) { SDLFAIL("SDL_Init"); }
  SDL_GL_SetSwapInterval(1);

  window = SDL_CreateWindow("Hello World",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
                            );
  if (window == nullptr) { SDLFAIL("SDL_CreateWindow"); }
  
  renderer = std::unique_ptr<Renderer>(new Renderer(window));

#ifdef EMSCRIPTEN
  // 0 fps means to use requestAnimationFrame; non-0 means to use setTimeout.
  emscripten_set_main_loop(main_loop, 0, 1);
#else
  while (main_loop_running) {
    main_loop();
    SDL_Delay(16);
  }
#endif
  
  SDL_DestroyWindow(window);
  SDL_Quit();
}
