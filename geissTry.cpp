//g++ geissTry.cpp -o geissTry.xps0 -lgeis -fpermissive -lX11
#include <iostream>
#include <X11/Xlib.h>
#include <geis/geis.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

using std::cout;
using std::endl;

//get current window id
static Window getRootWindow() {
  Display *display;
  Window focus;
  int revert;
  display = XOpenDisplay(NULL);
  XGetInputFocus(display, &focus, &revert);
  return focus;
}

static void gesture_added(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture added " << (int)gesture_type << endl;
}

static void gesture_removed(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture removed" << endl;
}

static void gesture_start(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture start" << endl;
}

static void gesture_update(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture update " << attrs[7].string_val << endl;
}

static void gesture_finish(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture finish" << endl;
}

GeisGestureFuncs gesture_funcs = {
    gesture_added,
    gesture_removed,
    gesture_start,
    gesture_update,
    gesture_finish
};

int main(int argc, char**argv) {

  //window info
  GeisXcbWinInfo xcb_win_info = {
    .display_name = NULL,
    .screenp = NULL,
    .window_id = getRootWindow()
  };
  GeisWinInfo win_info = {
    GEIS_XCB_FULL_WINDOW,
    &xcb_win_info
  };
  cout << std::hex << "using window 0x" << xcb_win_info.window_id << endl;

  GeisStatus status = GEIS_UNKNOWN_ERROR;
  GeisInstance instance;
  status = geis_init(&win_info, &instance);
  if (status != GEIS_STATUS_SUCCESS) {
    fprintf(stderr, "error in geis_init\n");
    return 1;
  }

  status = geis_configuration_supported(instance, GEIS_CONFIG_UNIX_FD);
  if (status != GEIS_STATUS_SUCCESS) {
    fprintf(stderr, "GEIS does not support Unix fd\n");
    return 1;
  }

  int fd = -1;
  status =
    geis_configuration_get_value(instance, GEIS_CONFIG_UNIX_FD, &fd);
  if (status != GEIS_STATUS_SUCCESS) {
    fprintf(stderr, "error retrieving GEIS fd\n");
    return 1;
  }

  char* gestures_list[20] = {NULL};
  gestures_list[0] = (char*)malloc(sizeof(char[16]));
  sprintf(gestures_list[0],"Drag,touch=2");
  gestures_list[1] = (char*)malloc(sizeof(char[16]));
  sprintf(gestures_list[1],"Pinch,touch=2");

  status = geis_subscribe(instance, GEIS_ALL_INPUT_DEVICES, (const char**)gestures_list, &gesture_funcs, NULL);
  if (status != GEIS_STATUS_SUCCESS) {
    fprintf(stderr, "error subscribing to gestures\n");
    return 1;
  }

  while(1) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    int sstat = select(fd + 1, &read_fds, NULL, NULL, NULL);
    if (sstat < 0) {
      fprintf(stderr, "error %d in select(): %s\n", errno, strerror(errno));
      break;
    }
    
    if (FD_ISSET(fd, &read_fds)) {
      geis_event_dispatch(instance);
    }       
  }

  geis_finish(instance);
  cout << "exit properly" << endl;
  return 0;
} 
