/* run opengl with multi touch in gtk3
 */

//gtk
#include<gtk/gtk.h>
#include<gdk/gdkx.h>

//glx
#include<X11/Xlib.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glu.h>

#include <iostream>
using namespace std;

static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display;
static GLXContext X_context;
static XVisualInfo *X_visual;
static XWindowAttributes X_attributes;

static void configureGL(GtkWidget *da, gpointer data) {
  static GLint attributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

  cout << "configure gl" << endl;
  DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

  X_window=gdk_x11_window_get_xid(GDK_WINDOW(DrawingWindow));
  X_display=gdk_x11_get_default_xdisplay();
  X_visual=glXChooseVisual(X_display, 0, attributes);
  X_context=glXCreateContext(X_display, X_visual, NULL, GL_TRUE);

  XGetWindowAttributes(X_display, X_window, &X_attributes);
  glXMakeCurrent(X_display, X_window, X_context);
  XMapWindow(X_display, X_window);

  cout << "Viewport " << X_attributes.width << " " << X_attributes.height << endl;

  glViewport(0, 0, X_attributes.width, X_attributes.height);
  glOrtho(-10,10,-10,10,-10,10);
  glScalef(5.0, 5.0, 5.0);
}

static void drawGL(GtkWidget *da, gpointer data) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  //Axis lines
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.8, 0.0, 0.0);
  glEnd();
  glXSwapBuffers(X_display, X_window);
}

gboolean on_touch(GtkWidget *widget, GdkEventTouch *event) {
  switch(event->type) {
  case GDK_TOUCH_BEGIN:
    cout << "Touch begin" << endl;
    break;
  case GDK_TOUCH_UPDATE:
    cout << "Touch update" << endl;
    break;
  case GDK_TOUCH_END:
    cout << "Touch end" << endl;
    break;
  case GDK_TOUCH_CANCEL:
    cout << "Touch cancel" << endl;
    break;
  default:
    cout << "Unknown touch type " << endl;
  }
  return false;
}

void close_program() {
   gtk_main_quit();
}

int main(int argc, char *argv[]) {
  //create window
  GtkWidget *window;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //drawing area new
  GtkWidget *da=NULL;
  da=gtk_drawing_area_new();
  gtk_widget_set_double_buffered(da, FALSE);
  gtk_container_add(GTK_CONTAINER(window), da);
  //gl
  g_signal_connect(da, "configure-event", G_CALLBACK(configureGL), NULL);
  g_signal_connect(da, "draw", G_CALLBACK(drawGL), NULL);

  //touch signal and set event
  g_signal_connect(window, "touch-event", G_CALLBACK(on_touch), NULL);
  gtk_widget_set_events(window, GDK_TOUCH_MASK);

  //make sure to close program
  g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

  //show window and run
  gtk_widget_show_all(window);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
