// Prints what SDL sees of joystick 0 -- GUID, counts, and a live event dump.
// Not shipped. Build: gcc -o jsdump tools/dev/jsdump.c $(sdl2-config --cflags --libs)
//
// The GUID is what a controller mapping has to be keyed on, and the event dump
// tells whether a dpad arrives as a hat, as buttons, or as axes.
#include <SDL.h>
#include <stdio.h>

int
main(int argc, char** argv)
{
  int seconds = argc > 1 ? SDL_atoi(argv[1]) : 5;

  SDL_Init(SDL_INIT_JOYSTICK);
  if (SDL_NumJoysticks() < 1) {
    printf("no joystick\n");
    return 1;
  }

  SDL_Joystick* js = SDL_JoystickOpen(0);
  char guid[64];
  SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(js), guid, sizeof(guid));

  printf("name: %s\n", SDL_JoystickName(js));
  printf("guid: %s\n", guid);
  printf("buttons: %d  hats: %d  axes: %d\n", SDL_JoystickNumButtons(js),
         SDL_JoystickNumHats(js), SDL_JoystickNumAxes(js));

  char* mapping = SDL_GameControllerMappingForGUID(SDL_JoystickGetGUID(js));
  printf("mapping: %s\n", mapping ? mapping : "(none)");
  if (mapping) SDL_free(mapping);
  fflush(stdout);

  Uint32 end = SDL_GetTicks() + seconds * 1000;
  while (SDL_GetTicks() < end) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_JOYHATMOTION)
        printf("hat %d = 0x%02x\n", ev.jhat.hat, ev.jhat.value);
      else if (ev.type == SDL_JOYBUTTONDOWN || ev.type == SDL_JOYBUTTONUP)
        printf("button %d = %d\n", ev.jbutton.button, ev.jbutton.state);
      else if (ev.type == SDL_JOYAXISMOTION)
        printf("axis %d = %d\n", ev.jaxis.axis, ev.jaxis.value);
      fflush(stdout);
    }
    SDL_Delay(10);
  }

  SDL_JoystickClose(js);
  SDL_Quit();
  return 0;
}
