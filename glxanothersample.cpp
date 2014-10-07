//gcc -Wall testGL.c -o testGL -lGL -lGLU -lX11 -lm `pkg-config --cflags --libs gtk+-3.0 gdk-x11-3.0`

#include<X11/Xlib.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<stdio.h>

static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display;
static GLXContext X_context;
static XVisualInfo *X_visual;
static XWindowAttributes X_attributes;
static GLint attributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
static float ang=0.0;
static guint timer_id;

static void drawGL(GtkWidget *da, gpointer data)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glRotatef(ang, 1.0, 1.0, 0.0); 
    glShadeModel(GL_FLAT);
  
    //Axis lines
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.8, 0.0, 0.0);
    glEnd();
   
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.8, 0.0);
    glEnd();
   
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.8);
    glEnd();
   
    glPopMatrix ();
    glXSwapBuffers(X_display, X_window);
}
static void configureGL(GtkWidget *da, gpointer data)
{
   printf("Congigure GL\n");
   DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

   X_window=gdk_x11_window_get_xid(GDK_WINDOW(DrawingWindow));
   X_display=gdk_x11_get_default_xdisplay();
   X_visual=glXChooseVisual(X_display, 0, attributes);
   X_context=glXCreateContext(X_display, X_visual, NULL, GL_TRUE);

   XGetWindowAttributes(X_display, X_window, &X_attributes);
   glXMakeCurrent(X_display, X_window, X_context);
   XMapWindow(X_display, X_window);
   printf("Viewport %i %i\n", (int)X_attributes.width, (int)X_attributes.height);
   glViewport(0, 0, X_attributes.width, X_attributes.height);
   glOrtho(-10,10,-10,10,-10,10);
   glScalef(5.0, 5.0, 5.0);
}
static gboolean rotate(gpointer data)
{
   ang++;
   gtk_widget_queue_draw_area(GTK_WIDGET(da), 0, 0, 500, 500);  
   return TRUE;
}
void close_program()
{
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   printf("Quit Program\n");
   gtk_main_quit();
}
int main(int argc, char **argv)
{
   gtk_init(&argc, &argv);
   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
   da=gtk_drawing_area_new();
   gtk_widget_set_double_buffered(da, FALSE);

   gtk_container_add(GTK_CONTAINER(window), da);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   gtk_widget_show(window);

   g_signal_connect(da, "configure-event", G_CALLBACK(configureGL), NULL);
   g_signal_connect(da, "draw", G_CALLBACK(drawGL), NULL);

   gtk_widget_show_all(window);

   timer_id=g_timeout_add(1000/60, rotate, da);

   gtk_main();
   return 0;
}
