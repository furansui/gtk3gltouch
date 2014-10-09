#include <iostream>
#include <gtk/gtk.h>
#include <libgrip/gripgesturemanager.h>

using std::cout;
using std::endl;

static GtkWidget* da; //drawing area

void close_program() {
   gtk_main_quit();
}

//BEGIN GTK Events
static void gesture_callback(GtkWidget* widget, GripTimeType time_type, GripGestureEvent* event, gpointer user_data) {
  cout << "gesture callback " << endl;
}

gboolean draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data) {
  guint width, height;
  GdkRGBA color;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);
  cairo_arc (cr,
             width / 2.0, height / 2.0,
             MIN (width, height) / 2.0,
             0, 2 * G_PI);

  gtk_style_context_get_color (gtk_widget_get_style_context (widget), GTK_STATE_FLAG_NORMAL, &color);
  gdk_cairo_set_source_rgba (cr, &color);

  cairo_fill (cr);

 return FALSE;
}

int main(int argc, char *argv[]) {
  //create window
  GtkWidget *window;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "LibGrip Example");


  //add drawing area with masks
  da = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), da);
  gtk_widget_add_events(da, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK);  
  g_signal_connect (G_OBJECT (da), "draw", G_CALLBACK (draw_callback), NULL);

  //libgrip to drawing area
  GripGestureManager* manager;
  manager = grip_gesture_manager_get();
  grip_gesture_manager_register_window(manager, da, GRIP_GESTURE_PINCH, GRIP_DEVICE_TOUCHSCREEN, 2, gesture_callback, NULL, NULL);

  GripInputDevice* device;
  

  //make sure to close program
  g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

  //show window and run
  gtk_widget_show_all(window);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
