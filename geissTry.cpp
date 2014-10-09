//g++ geissTry.cpp -o geissTry.xps0 -lgeis -fpermissive -lX11
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
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

std::string gestures[] = {"Drag,touch=2","Pinch,touch=2"};

struct GestureLog {
  long int timestamp;
  double focusx[2];
  int touches;
  std::pair<int,int> boundingbox[2];
  int position[2];
  std::vector<int> touchid;
  std::vector<std::pair<double,double> > touchcoord;
} gestureLog;

static void gesture_start(GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, GeisGestureAttr * attrs, bool endGesture) 
{
  gestureLog.touchid.clear();
  gestureLog.touchcoord.clear();

}

static void gesture_match(GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, GeisGestureAttr * attrs, bool endGesture) 
{
  for(int i=0;i<2;i++) {
    if(strcmp(gestures[i].c_str(),attrs[7].string_val) == 0) {
      //cout << "this is " << gestures[i] << endl;
    }
  }

  gestureLog.boundingbox[0].first = attrs[9].float_val;
  gestureLog.boundingbox[0].second = attrs[10].float_val;
  gestureLog.boundingbox[1].first = attrs[11].float_val;
  gestureLog.boundingbox[1].second = attrs[12].float_val;
  
  cout << std::setw(10);
  cout << gestureLog.boundingbox[0].first << " ";
  cout << gestureLog.boundingbox[0].second << " ";
  cout << gestureLog.boundingbox[1].first << " ";
  cout << gestureLog.boundingbox[1].second << " ";
  cout << endl;
}

static void printAttr(GeisGestureAttr * attr) {
  cout << "attr " << attr->name << " = ";
  switch (attr->type) {
  case GEIS_ATTR_TYPE_BOOLEAN:
    cout << attr->boolean_val;break;
  case GEIS_ATTR_TYPE_FLOAT:
    cout <<  attr->float_val;
    break;
  case GEIS_ATTR_TYPE_INTEGER:
    cout << attr->integer_val;
    break;
  case GEIS_ATTR_TYPE_STRING:
    cout << attr->string_val;
    break;
  default:
    break;
  }
  cout << endl;
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
  gesture_start(gesture_type, gesture_id, attr_count, attrs, false);
  for(int i=0;i<attr_count;i++) {
    cout << std::dec << i << " ";
    printAttr(&attrs[i]);
  }
}

static void gesture_update(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  gesture_match(gesture_type, gesture_id, attr_count, attrs, false);
}

static void gesture_finish(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture finish" << endl;
  gesture_match(gesture_type, gesture_id, attr_count, attrs, true);
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
  for(int i=0;i<2;i++) {
    gestures_list[i] = (char*)malloc(sizeof(char[16]));
    sprintf(gestures_list[i],"%s",gestures[i].c_str());
  }

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
