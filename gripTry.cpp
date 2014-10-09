#include <gtk/gtk.h>
#include <libgrip/gripgesturemanager.h>

void close_program() {
   gtk_main_quit();
}

int main(int argc, char *argv[]) {
  //create window
  GtkWidget *window;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  //make sure to close program
  g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

  //show window and run
  gtk_widget_show_all(window);
  gtk_widget_show(window);
  gtk_main();
  return 0;
}
