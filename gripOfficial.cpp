#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <libgrip/gripgesturemanager.h>

static gint    rotate = 0;
static gdouble scale = 1.0;
static gdouble translate[2] = { 200, 200 };
static gint    in_touch = 0;

static void subscribe_gestures(GripGestureManager *manager, GtkWidget *window, GtkWidget *da);

struct app_data {
  GripGestureManager *manager;
  GtkWidget *main_window;
  GtkWidget *da;
};

static gboolean
draw_canvas (GtkWidget *widget,
             cairo_t   *cr,
             gpointer   data)
{
  gdouble radians;
  gint width = (in_touch > 0) ? 10 : 1;

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, width);

  radians = rotate * (G_PI / 180);
  cairo_translate (cr, translate[0], translate[1]);
  cairo_scale (cr, scale, scale);
  cairo_rotate (cr, radians);

  cairo_rectangle (cr, -50, -50, 100, 100);
  cairo_stroke_preserve (cr);
  cairo_set_source_rgb (cr, 1, 0, 1);
  cairo_fill (cr);

  return FALSE;
}

static void
gesture_callback (GtkWidget        *widget,
                  GripTimeType      time_type,
                  GripGestureEvent  *event,
                  gpointer           user_data)
{
  printf("gesture callback\n");
  if (time_type == GRIP_TIME_START)
    {
      in_touch++;
    }
  else if (time_type == GRIP_TIME_END)
    {
      in_touch--;
    }
  else
    {
      switch (event->type)
        {
        case GRIP_GESTURE_DRAG:
          {
            GripEventGestureDrag *e = (GripEventGestureDrag *)event;

            translate[0] += e->delta_x;
            translate[1] += e->delta_y;
          }
          break;

        case GRIP_GESTURE_PINCH:
          {
            GripEventGesturePinch *e = (GripEventGesturePinch *)event;
            GtkWindow *toplevel = (GtkWindow *)gtk_widget_get_toplevel (widget);
            gint window_width, window_height;
            const GripAxisExtents *x;
            const GripAxisExtents *y;
            gfloat width = 0;
            gfloat height = 0;
            gfloat factor = 0;

            gtk_window_get_size (toplevel, &window_width, &window_height);
            x = grip_input_device_get_x_extents (e->input_device);
            y = grip_input_device_get_y_extents (e->input_device);
            width = window_width / (x->maximum - x->minimum);
            height = window_height / (y->maximum - y->minimum);
            factor = sqrtf (width * width + height * height) / 5.2723;

            scale += e->radius_delta * factor;
          }
          break;

        case GRIP_GESTURE_ROTATE:
          {
            GripEventGestureRotate *e = (GripEventGestureRotate *)event;

            rotate += e->angle_delta * 100;
          }
          break;

        case GRIP_GESTURE_TAP:
          break;
        }
    }

  gtk_widget_queue_draw (GTK_WIDGET (widget));
}

static void
subscribe_toggled(GtkToggleButton *toggle_button, gpointer user_data)
{
  struct app_data *app = (struct app_data*)(user_data);
  if (gtk_toggle_button_get_active(toggle_button)) {
      subscribe_gestures(app->manager, app->main_window, app->da);
  } else {
      grip_gesture_manager_unregister_window(app->manager, app->main_window);
  }
}

static GtkTreeModel *
create_model ()
{
  return (GtkTreeModel *)gtk_list_store_new (1, G_TYPE_STRING);
}

/* Change this to suit your needs. */
#define DEVICE_TYPE GRIP_DEVICE_TOUCHSCREEN

static void
subscribe_gestures(GripGestureManager *manager, GtkWidget *window,
    GtkWidget *da)
{
  grip_gesture_manager_register_window (manager,
                                        da,
                                        GRIP_GESTURE_PINCH,
                                        DEVICE_TYPE,
                                        2,
                                        gesture_callback,
                                        NULL, NULL);

  grip_gesture_manager_register_window (manager,
                                        da,
                                        GRIP_GESTURE_ROTATE,
                                        DEVICE_TYPE,
                                        2,
                                        gesture_callback,
                                        NULL, NULL);

  grip_gesture_manager_register_window (manager,
                                        da,
                                        GRIP_GESTURE_DRAG,
                                        DEVICE_TYPE,
                                        2,
                                        gesture_callback,
                                        NULL, NULL);

  grip_gesture_manager_register_window (manager,
                                        da,
                                        GRIP_GESTURE_TAP,
                                        DEVICE_TYPE,
                                        2,
                                        gesture_callback,
                                        NULL, NULL);
}

static GtkWidget *
create_window (struct app_data *app)
{
  app->main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GtkWidget *window = app->main_window;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *subscribed_toggle;
  GtkWidget *tv;
  GtkTreeModel *model;
  GtkWidget *sw;
  const GdkRGBA white = { 0xffff, 0xffff, 0xffff, 0xffff };

  app->da = gtk_drawing_area_new ();

  gtk_window_set_title (GTK_WINDOW (window), "Touch Demo");
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 600);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), app->da, TRUE, TRUE, 0);

  model = create_model ();
  sw = gtk_scrolled_window_new (NULL, NULL);
  tv = gtk_tree_view_new_with_model (model);
  g_object_unref (model);

  gtk_container_add (GTK_CONTAINER (sw), tv);

  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Quit");
  g_signal_connect (button,
                    "clicked",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  subscribed_toggle = gtk_toggle_button_new_with_label ("Gestures");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(subscribed_toggle), TRUE);
  g_signal_connect (subscribed_toggle,
                    "toggled",
                    G_CALLBACK (subscribe_toggled),
                    app);
  gtk_box_pack_start (GTK_BOX (vbox), subscribed_toggle, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  gtk_container_add (GTK_CONTAINER (window), hbox);

  gtk_widget_override_background_color (app->da, GTK_STATE_FLAG_NORMAL, &white);

  g_signal_connect (app->da, "draw",
                    G_CALLBACK (draw_canvas), NULL);

  subscribe_gestures(app->manager, window, app->da);
  return window;
}

static void
abort_handler (int x)
{
  g_print (" **** ABORT ****\n");

  exit (1);
}

int
main (int argc, char **argv)
{
  struct app_data app;

  gtk_init (&argc, &argv);

  /* Don't crash X if we're using some shitty Intel graphics like
   * my Dell XT2 has in it. */
  signal (SIGABRT, abort_handler);

  app.manager = grip_gesture_manager_get ();
  create_window (&app);

  g_signal_connect (app.main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (GTK_WIDGET (app.main_window));

  gtk_main ();

  return 0;
}
