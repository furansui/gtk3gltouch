//g++ gtk2geis.cpp -o gtk2geis.xps0 `pkg-config --cflags --libs gtkmm-2.4` -lgeis -lX11
#include <gtkmm.h>
#include <iostream>
#include <errno.h>
#include <X11/Xlib.h>
#include <geis/geis.h>
#include <gdk/gdkx.h> //xid
#include <helium/thread/thread.h>

using std::endl;
using std::cout;

const std::string gesturesString[] = {"Drag,touch=2","Pinch,touch=2"};

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
  //gesture_start(gesture_type, gesture_id, attr_count, attrs, false);
  //for(int i=0;i<attr_count;i++) {
  //  cout << std::dec << i << " ";
  //  printAttr(&attrs[i]);
  //}
}

static void gesture_update(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  //gesture_match(gesture_type, gesture_id, attr_count, attrs, false);
}

static void gesture_finish(void *cookie,
			  GeisGestureType gesture_type,
			  GeisGestureId gesture_id,
			  GeisSize attr_count, 
			  GeisGestureAttr * attrs)
{
  cout << "gesture finish" << endl;
  //gesture_match(gesture_type, gesture_id, attr_count, attrs, true);
}


GeisGestureFuncs gestureFuncs = {
    gesture_added,
    gesture_removed,
    gesture_start,
    gesture_update,
    gesture_finish
};


class Spinner:public helium::Thread{
  bool &shouldExit;
  GeisXcbWinInfo xcb_win_info;
  GeisWinInfo win_info;
  GeisStatus status;
  GeisInstance instance;
  int fd;
  char* gesturesList[20];
public:  
  Spinner(bool &pshouldExit):
    shouldExit(pshouldExit),
    status(GEIS_UNKNOWN_ERROR)
  {
  }
  void setGeis(Window w) {
    //window info
    xcb_win_info = {
      .display_name = NULL,
      .screenp = NULL,
      .window_id = w
    };
    win_info = {
      GEIS_XCB_FULL_WINDOW,
      &xcb_win_info
    };
    cout << std::hex << "using window 0x" << xcb_win_info.window_id << endl;

    status = geis_init(&win_info, &instance);
    if (status != GEIS_STATUS_SUCCESS) {
      fprintf(stderr, "error in geis_init\n");
      throw;
    }
    status = geis_configuration_supported(instance, GEIS_CONFIG_UNIX_FD);
    if (status != GEIS_STATUS_SUCCESS) {
      fprintf(stderr, "GEIS does not support Unix fd\n");
      throw;
    }
    
    status =
      geis_configuration_get_value(instance, GEIS_CONFIG_UNIX_FD, &fd);
    if (status != GEIS_STATUS_SUCCESS) {
      fprintf(stderr, "error retrieving GEIS fd\n");
      throw;
    }

    for(int i=0;i<20;i++)
      gesturesList[i] = NULL;
    for(int i=0;i<2;i++) {
      gesturesList[i] = (char*)malloc(sizeof(char[16]));
      sprintf(gesturesList[i],"%s",gesturesString[i].c_str());
    }

    status = geis_subscribe(instance, GEIS_ALL_INPUT_DEVICES, (const char**)gesturesList, &gestureFuncs, NULL);
    if (status != GEIS_STATUS_SUCCESS) {
      fprintf(stderr, "error subscribing to gestures\n");
      throw;
    }

    start();
  }
  void run(){
    while(1) {
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(fd, &read_fds);
      int sstat = select(fd + 1, &read_fds, NULL, NULL, NULL);
      if (sstat < 0) {
	fprintf(stderr, "error %d in select(): %s\n", errno, strerror(errno));
	break;
      }     

      if(shouldExit) {
	cout << "exiting thread" << endl;
	break;
      }
      cout << ".";

      if (FD_ISSET(fd, &read_fds)) {
	geis_event_dispatch(instance);
      }       
    }//while
    geis_finish(instance);
  }
};

class HelloWorld : public Gtk::Window {
public:
  HelloWorld():
    label("Hello"),
    shouldExit(false),
    spin(shouldExit)
  {
    v.pack_start(label);
    add(v);
    show_all_children();
    show();
    cout << "initialize 0x" << std::hex << GDK_WINDOW_XID(this->get_window()->gobj()) << endl;

    signal_hide().connect(sigc::mem_fun(*this,&HelloWorld::onExit));

    spin.setGeis(GDK_WINDOW_XID(this->get_window()->gobj()));
  }

private:
  //Member widgets:
  Gtk::Label label;
  Gtk::VBox v;
  
  Spinner spin;
  bool shouldExit;

  void onExit() {
    cout << "window is exiting" << endl;
    shouldExit = true;
  }

};


int main(int argc, char *argv[]) {
  Gtk::Main kit(argc, argv);

  HelloWorld helloWorld;
  Gtk::Main::run(helloWorld);
  return 0; 
} 
