/* run opengl with multi touch in gtk3
 */

#include <gtk/gtk.h>
#include <iostream>
using namespace std;

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


int main(int argc, char *argv[]) {
  //create window
  GtkWidget *window;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //touch signal and set event
  g_signal_connect(window, "touch-event", G_CALLBACK(on_touch), NULL);
  gtk_widget_set_events(window, GDK_TOUCH_MASK);

  //show window and run
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
