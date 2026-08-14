// Rufe.org LLC 2022-2025: ISC License

enum { JOYSTICK_VERBOSE = 0 };
enum { BIND_VERBOSE = 0 };

// game controllers & joysticks
DATA SDL_Joystick* joystick_ptrD;
DATA float jxD;
DATA float jyD;
DATA int rtrigger_heldD;
DATA int joystick_refcountD;

enum { CHAR_LTRIGGER = '0' };
enum { CHAR_RTRIGGER = '-' };  // gameplay: zoom, menu: sort
enum { MAX_MAPPING = 64 };
enum { MAX_AXIS = 16 };

enum {
  JS_SOUTH,
  JS_EAST,
  JS_WEST,
  JS_NORTH,
  JS_LSHOULDER,
  JS_RSHOULDER,
  JS_LTRIGGER,  // Disabled if AXIS trigger support exists
  JS_RTRIGGER,  // Disabled if AXIS trigger support exists
  JS_BACK,
  JS_START,
  JS_DPUP,
  JS_DPDOWN,
  JS_DPLEFT,
  JS_DPRIGHT,
  JS_COUNT,
};
DATA char mappingD[MAX_MAPPING];

// SDL_HAT_* is absent from the flattened SDL header; these are the SDL2 ABI
// values, reused as the internal dpad bitmask.
enum { HAT_UP = 0x01, HAT_RIGHT = 0x02, HAT_DOWN = 0x04, HAT_LEFT = 0x08 };
DATA int dpad_bitsD;
DATA int dpad_bit_by_idD[] = {HAT_UP, HAT_DOWN, HAT_LEFT, HAT_RIGHT};
DATA int modifier_heldD;

enum {
  JA_LX,
  JA_LY,
  JA_LTRIGGER,
  JA_RX,
  JA_RY,
  JA_RTRIGGER,
  JA_COUNT,
};
DATA char ja_mappingD[MAX_AXIS];

STATIC int
joystick_enabled()
{
  return joystick_ptrD != 0;
}
// TBD: deadzone check?
STATIC int
joystick_2f(float* x, float* y)
{
  *x = jxD;
  *y = jyD;
  return 0;
}
STATIC int
joystick_count()
{
  return SDL_NumJoysticks();
}

#define BUTTON(text, id)                                                   \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":b");                             \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = parse_num(fa);                                              \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(mappingD)) mappingD[kv] = id;                            \
    }                                                                      \
  }
#define AXIS(text, id)                                                     \
  {                                                                        \
    char* fa = strstr(mapping, "," text ":a");                             \
    if (BIND_VERBOSE) Log("%s", fa);                                       \
    if (fa) {                                                              \
      fa += AL(text) + 2;                                                  \
      int kv = parse_num(fa);                                              \
      if (BIND_VERBOSE) Log(text " is button %d (%c) -> %d", kv, *fa, id); \
      if (kv < AL(ja_mappingD)) ja_mappingD[kv] = id;                      \
    }                                                                      \
  }
STATIC int
find_axis(ja)
{
  for (int it = 0; it < AL(ja_mappingD); ++it)
    if (ja_mappingD[it] == ja) return 1;
  return 0;
}
STATIC int
joystick_assign(jsidx)
{
  if (joystick_ptrD) SDL_JoystickClose(joystick_ptrD);

  memset(mappingD, -1, sizeof(mappingD));
  memset(ja_mappingD, -1, sizeof(ja_mappingD));

  void* joystick = 0;
  if (jsidx >= 0) joystick = SDL_JoystickOpen(jsidx);

  char* mapping = 0;
  int product = 0;
  if (joystick) {
    const char* name = SDL_JoystickNameForIndex(jsidx);
    product = SDL_JoystickGetDeviceProduct(jsidx);
    Log("joystick_assign (product 0x%x): %s", product, name);
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(joystick);
    mapping = SDL_GameControllerMappingForGUID(guid);
    if (!mapping) {
      if (BIND_VERBOSE) Log("no mapping for joystick");
      SDL_JoystickClose(joystick);
      joystick = 0;
    }
  }
  joystick_ptrD = joystick;
  // Dynamic assignment of menu selection mode
  // Controller hotplugging can toggle this feature
  platformD.selection = joystick ? fnptr(touch_selection) : noop;

  if (mapping) {
    // TBD: hacky zoom adjustment for devices using a controller
    globalD.zoom_factor = 1;

    Log("GUID mapping: %s", mapping);
    BUTTON("a", JS_SOUTH);
    BUTTON("b", JS_EAST);
    BUTTON("x", JS_WEST);
    BUTTON("y", JS_NORTH);
    BUTTON("leftshoulder", JS_LSHOULDER);
    BUTTON("rightshoulder", JS_RSHOULDER);
    BUTTON("back", JS_BACK);
    BUTTON("start", JS_START);
    BUTTON("dpup", JS_DPUP);
    BUTTON("dpdown", JS_DPDOWN);
    BUTTON("dpleft", JS_DPLEFT);
    BUTTON("dpright", JS_DPRIGHT);

    AXIS("leftx", JA_LX);
    AXIS("lefty", JA_LY);
    AXIS("lefttrigger", JA_LTRIGGER);
    AXIS("rightx", JA_RX);
    AXIS("righty", JA_RY);
    AXIS("righttrigger", JA_RTRIGGER);

    if (!find_axis(JA_LTRIGGER)) BUTTON("lefttrigger", JS_LTRIGGER);
    if (!find_axis(JA_RTRIGGER)) BUTTON("righttrigger", JS_RTRIGGER);

    SDL_free(mapping);
  }

  // Center input
  jxD = jyD = .5;
  dpad_bitsD = 0;
  modifier_heldD = 0;
}
STATIC int
joystick_dir()
{
  int scale = 64;
  int x = jxD * (PADSIZE - scale) + scale / 2;
  int y = jyD * (PADSIZE - scale) + scale / 2;

  int n = dpad_nearest_pp(y, x, 0);
  return (pp_keyD[n]);
}
STATIC int
joystick_button(button)
{
  char c = key_dir(joystick_dir());
  if (c == ' ')
    c = (button == JS_EAST) ? 'a' : '.';
  else if (button == JS_EAST)
    c &= ~0x20;  // run
  return c;
}

int
sdl_axis_motion(SDL_Event event)
{
  USE(mode);
  int ret = 0;

  if (JOYSTICK) {
    int axis = event.jaxis.axis;

    if (JOYSTICK_VERBOSE)
      Log("axis raw %d value %d", event.jaxis.axis, event.jaxis.value);

    if (axis < AL(ja_mappingD))
      axis = ja_mappingD[axis];
    else
      axis = -1;

    int ok = event.jaxis.value + 32768;
    float norm = (float)ok / (32767 * 2 + 1);
    if (JOYSTICK_VERBOSE) Log("norm %.03f", norm);

    int trigger = (event.jaxis.value > 10000);
    switch (axis) {
      case JA_LX:
        jxD = norm;
        break;
      case JA_LY:
        jyD = norm;
        break;
      case JA_LTRIGGER:
        // Gameplay spends the left trigger on the modifier layer instead.
        if (mode != 0 && trigger && !modifier_heldD) ret = CHAR_LTRIGGER;
        modifier_heldD = trigger;
        break;
      case JA_RX:
      case JA_RY:
        break;
      case JA_RTRIGGER:
        if (trigger && !rtrigger_heldD) ret = mode == 0 ? 'x' : CHAR_RTRIGGER;
        rtrigger_heldD = trigger;
        break;
    }
  }
  return ret;
}

int
overlay_dir(dir, finger)
{
  int dx = dir_x(dir);
  int dy = dir_y(dir);

  if (!dx && !dy) {
    // Controller uses center tap as study & confirm
    return (finger ? 'A' : 'a') + finger_rowD;
  }

  if (dx && !dy) {
    if (finger)
      finger_rowD = dx < 0 ? overlay_begin() : overlay_end();
    else
      return column_transition(finger_colD, dx);
  }
  if (dy && !dx) {
    if (finger)
      finger_rowD = overlay_bisect(dy);
    else
      finger_rowD = overlay_input(dy);
  }
  return CTRL('d');
}
// A handheld dpad arrives as a hat or as buttons, while upstream derives
// direction from the analog axes alone; both encodings fold into jx/jy here.
// The direction is kept as held state so a shoulder or B press still applies to
// it, and a press also acts at once, matching the touch pad on mobile.
STATIC int
dpad_direction(bits)
{
  int col = ((bits & HAT_RIGHT) != 0) - ((bits & HAT_LEFT) != 0);
  int row = ((bits & HAT_DOWN) != 0) - ((bits & HAT_UP) != 0);

  jxD = .5f + .5f * col;
  jyD = .5f + .5f * row;

  return joystick_dir();
}
STATIC int
dpad_input(dir, mode)
{
  if (mode == 0) {
    char c = key_dir(dir);
    // Held trigger runs rather than steps: the same uppercase the game already
    // reads from B with a direction, but without the step the press costs.
    if (modifier_heldD && c != ' ') c &= ~0x20;
    return c;
  }
  if (mode == 1) return overlay_dir(dir, 0);
  return ' ';  // popup dismissal, as a touch on the pad does
}
int
sdl_hat_motion(SDL_Event event)
{
  USE(mode);

  int prev = dpad_bitsD;
  dpad_bitsD = event.jhat.value;

  int ret = 0;
  int dir = dpad_direction(dpad_bitsD);
  // Releases only recenter; acting on them too would double every step.
  if (dpad_bitsD & ~prev) ret = dpad_input(dir, mode);
  if (ret > ' ' && mode == 0 && msg_moreD) ret = ' ';
  return ret;
}
// Held left trigger: the commands the ten buttons have no room for. A modifier
// rather than a stick click, since a device may carry no sticks at all.
STATIC int
joystick_modifier_button(button)
{
  switch (button) {
    case JS_SOUTH:
      return 'e';  // use equipment
    case JS_EAST:
      return 'd';  // drop an item
    case JS_WEST:
      return 'p';  // message history
    case JS_NORTH:
      return '?';  // help
    case JS_LSHOULDER:
      return CHAR_RTRIGGER;  // magnification
    case JS_RSHOULDER:
      return 'M';  // locate on the level map
    default:
      return 0;
  }
}
int
joystick_game_button(button)
{
  if (modifier_heldD) return joystick_modifier_button(button);

  switch (button) {
    case JS_SOUTH:
    case JS_EAST:  // movement
      return joystick_button(button);
    case JS_WEST:
      return 'R';  // rest
    case JS_NORTH:
      return '!';
    case JS_LSHOULDER:
      return 'c';  // character sheet
    case JS_RSHOULDER:
      return 'm';  // dungeon map
    case JS_LTRIGGER:
      return CHAR_LTRIGGER;
    case JS_RTRIGGER:
      return 'x';  // examine along a direction
    case JS_BACK:
      return CTRL('z');  // undo a turn
    case JS_START:
      return CTRL('w');  // show advanced menu
    default:
      return 0;
  }
}
int
joystick_menu_button(button)
{
  switch (button) {
    case JS_SOUTH:
      return overlay_dir(joystick_dir(), 0);
    case JS_NORTH:
      // Second-finger variant: inspect rather than use, and page scrolling.
      return overlay_dir(joystick_dir(), 1);
    case JS_EAST:
    case JS_WEST:
      return ESCAPE;
    default:
      return 0;
  }
}
int
joystick_popup_button(button)
{
  switch (button) {
    case JS_SOUTH:
    case JS_EAST:
      return ESCAPE;
    case JS_WEST:
      return 'p';
    case JS_NORTH:
      return 'o';  // from death screen, go back to last game frame; reroll
    case JS_LSHOULDER:
      return 'c';
    case JS_BACK:
      return CTRL('z');
    default:
      return 0;
  }
}
int
sdl_joystick_event(SDL_Event event)
{
  USE(mode);
  int button = event.jbutton.button;
  int state = event.jbutton.state;

  if (JOYSTICK_VERBOSE) {
    char* statename[] = {"release", "press"};
    Log("button %d %s mode %d", button, statename[state], mode);
  }

  int ret = 0;
  if (JOYSTICK) {
    int id = -1;
    if (button >= 0 && button < AL(mappingD)) id = mappingD[button];

    if (id == JS_LTRIGGER) {
      modifier_heldD = state;
      if (mode == 0) return 0;
    }

    if (id >= JS_DPUP && id <= JS_DPRIGHT) {
      int bit = dpad_bit_by_idD[id - JS_DPUP];
      if (state)
        dpad_bitsD |= bit;
      else
        dpad_bitsD &= ~bit;

      int dir = dpad_direction(dpad_bitsD);
      if (state) ret = dpad_input(dir, mode);
      if (ret > ' ' && mode == 0 && msg_moreD) ret = ' ';
      return ret;
    }

    if (state) {
      button = id;

      if (mode == 0) {
        ret = joystick_game_button(button);
        if (ret > ' ' && msg_moreD) ret = ' ';
      } else if (mode == 1) {
        ret = joystick_menu_button(button);
      } else if (mode == 2) {
        ret = joystick_popup_button(button);
      }
    } else {
      if (blipD) ret = ' ';
    }
  }
  return ret;
}

int
sdl_joystick_device(SDL_Event event)
{
  if (JOYSTICK) {
    int type = event.type;
    if (event.type == SDL_JOYDEVICEADDED) {
      joystick_assign(event.jdevice.which);
    }
    if (event.type == SDL_JOYDEVICEREMOVED) {
      joystick_assign(SDL_NumJoysticks() - 1);
    }
  }
}

STATIC int
joystick_init()
{
  int init = 0;
  if (globalD.use_joystick) {
    MUSE(global, label_button_order);
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS,
                label_button_order ? "1" : "0");

    init = (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0);
    joystick_refcountD += init;
  }
  return init;
}

STATIC int
joystick_update()
{
  joystick_assign(-1);
  while (joystick_refcountD--) SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
  joystick_init();
}

STATIC int
joystick_active()
{
  return joystick_ptrD != 0;
}
// The buttons are bound here, so the help for them is written here too. Returns
// zero when there is no pad, leaving the keyboard help to stand.
int
joystick_help()
{
  if (!joystick_active()) return 0;

  int line = 1;
  screen_submodeD = 1;

  BufMsg(screen, "dpad: step, L2+dpad: run");
  BufMsg(screen, "A: act on this square");
  BufMsg(screen, "B: use an item, B+dpad: run");
  BufMsg(screen, "X: rest");
  BufMsg(screen, "Y: repeat last spell/item");
  BufMsg(screen, "L1: character sheet, held");
  BufMsg(screen, "R1: dungeon map, held");
  BufMsg(screen, "R2: look along a direction");
  BufMsg(screen, "SELECT: undo a turn");
  BufMsg(screen, "START: game menu");
  line += 1;
  BufMsg(screen, "A takes stairs, picks an item up");
  BufMsg(screen, "or enters a shop where it can,");
  BufMsg(screen, "and searches where it cannot.");

  BufPad(screen, AL(screenD), 34);

  line = 1;
  BufMsg(screen, "HOLD L2");
  BufMsg(screen, "  A: use equipment");
  BufMsg(screen, "  B: drop an item");
  BufMsg(screen, "  X: message history");
  BufMsg(screen, "  Y: this help");
  BufMsg(screen, "  L1: zoom adjustment");
  BufMsg(screen, "  R1: locate on the map");
  line += 1;
  BufMsg(screen, "LISTS");
  BufMsg(screen, "  dpad: move the selection");
  BufMsg(screen, "  A: choose, B or X: back");
  BufMsg(screen, "  Y: inspect instead of use");
  BufMsg(screen, "  Y+dpad: page or jump to an end");

  return 1;
}

