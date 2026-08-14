// Virtual gamepad over uinput, for testing the dpad paths without hardware.
// Not shipped. Build: gcc -o vpad tools/dev/vpad.c
//
// The device exposes a hat (ABS_HAT0X/Y), a left stick and the eight face and
// shoulder buttons, plus BTN_DPAD_* so both dpad encodings can be exercised.
// Commands on stdin, one per line:
//   h <x> <y>    hat, each -1..1
//   b <code> <0|1>   key event by evdev code
//   a <code> <value> absolute axis by evdev code
//   q            quit (destroys the device)
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int fd;

static void
emit(int type, int code, int value)
{
  struct input_event ev = {.type = type, .code = code, .value = value};
  write(fd, &ev, sizeof(ev));
}

static void
sync_report()
{
  emit(EV_SYN, SYN_REPORT, 0);
}

int
main(int argc, char** argv)
{
  int keys[] = {BTN_SOUTH,     BTN_EAST,      BTN_NORTH,     BTN_WEST,
                BTN_TL,        BTN_TR,        BTN_TL2,       BTN_TR2,
                BTN_SELECT,    BTN_START,     BTN_DPAD_UP,   BTN_DPAD_DOWN,
                BTN_DPAD_LEFT, BTN_DPAD_RIGHT};
  int axes[] = {ABS_X, ABS_Y, ABS_Z, ABS_RZ, ABS_HAT0X, ABS_HAT0Y};

  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    perror("/dev/uinput");
    return 1;
  }

  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  for (int it = 0; it < sizeof(keys) / sizeof(*keys); ++it)
    ioctl(fd, UI_SET_KEYBIT, keys[it]);

  ioctl(fd, UI_SET_EVBIT, EV_ABS);
  for (int it = 0; it < sizeof(axes) / sizeof(*axes); ++it)
    ioctl(fd, UI_SET_ABSBIT, axes[it]);

  struct uinput_user_dev dev = {0};
  snprintf(dev.name, sizeof(dev.name), "%s",
           argc > 1 ? argv[1] : "moria virtual pad");
  dev.id.bustype = BUS_USB;
  dev.id.vendor = 0x1209;
  dev.id.product = 0x0001;
  dev.id.version = 1;
  for (int it = 0; it < sizeof(axes) / sizeof(*axes); ++it) {
    int hat = (axes[it] == ABS_HAT0X || axes[it] == ABS_HAT0Y);
    dev.absmin[axes[it]] = hat ? -1 : -32768;
    dev.absmax[axes[it]] = hat ? 1 : 32767;
  }

  write(fd, &dev, sizeof(dev));
  if (ioctl(fd, UI_DEV_CREATE) < 0) {
    perror("UI_DEV_CREATE");
    return 1;
  }
  printf("ready\n");
  fflush(stdout);

  char line[128];
  while (fgets(line, sizeof(line), stdin)) {
    int a, b;
    if (sscanf(line, "h %d %d", &a, &b) == 2) {
      emit(EV_ABS, ABS_HAT0X, a);
      emit(EV_ABS, ABS_HAT0Y, b);
      sync_report();
    } else if (sscanf(line, "b %d %d", &a, &b) == 2) {
      emit(EV_KEY, a, b);
      sync_report();
    } else if (sscanf(line, "a %d %d", &a, &b) == 2) {
      emit(EV_ABS, a, b);
      sync_report();
    } else if (line[0] == 'q') {
      break;
    }
    printf("ok\n");
    fflush(stdout);
  }

  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
  return 0;
}
