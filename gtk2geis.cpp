//g++ gtk2geis.cpp -o gtk2geis.xps0 `pkg-config --cflags --libs gtkmm-2.4` -lgeis
#include <gtkmm.h>
#include <iostream>

class HelloWorld : public Gtk::Window {
public:
  HelloWorld():
    label("Hello") {
    v.pack_start(label);
    add(v);
    show_all_children();
    show();
  }

protected:
  //Member widgets:
  Gtk::Label label;
Gtk::VBox v;
};



int main(int argc, char *argv[]) {
  Gtk::Main kit(argc, argv);

  HelloWorld helloWorld;
  Gtk::Main::run(helloWorld);
  return 0; 
} 
