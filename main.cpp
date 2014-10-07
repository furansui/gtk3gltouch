/* gtk3 touch source:
 * http://vbkaisetsu.sky-air.net/wordpress/2012/03/gtk-3-4-%E3%81%AE%E3%82%BF%E3%83%83%E3%83%81%E3%82%A4%E3%83%99%E3%83%B3%E3%83%88%E3%82%92%E8%A9%A6%E3%81%99/
 * 
 */

#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
  GtkWidget *window;
  gtk_init(&argc, &argv);
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_widget_show(window);
  gtk_main();

  return 0;
}
