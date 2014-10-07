#include <gtk/gtk.h>

typedef struct
{
    gdouble x, y;
    gdouble old_x, old_y;
    gdouble r, g, b;
    GdkEventSequence *seq;
} TouchData;

GList *touch_list = NULL;
cairo_surface_t *image = NULL;
cairo_t *icr;
GRand *randseed;

gboolean on_touch(GtkWidget *widget, GdkEventTouch *event)
{
    GList *lst;
    TouchData *data;
    switch(event->type)
    {
    case GDK_TOUCH_BEGIN:
        data = g_malloc(sizeof(TouchData));
        data->old_x = event->axes[0];
        data->old_y = event->axes[1];
        data->x = event->axes[0];
        data->y = event->axes[1];
        data->r = g_rand_double(randseed);
        data->g = g_rand_double(randseed);
        data->b = g_rand_double(randseed);
        data->seq = event->sequence;
        touch_list = g_list_append(touch_list, data);
        break;
    case GDK_TOUCH_UPDATE:
        lst = touch_list;
        while(lst)
        {
            data = (TouchData*)lst->data;
            if(data->seq == event->sequence)
            {
                data->old_x = data->x;
                data->old_y = data->y;
                data->x = event->axes[0];
                data->y = event->axes[1];
                gtk_widget_queue_draw(widget);
                break;
            }
            lst = lst->next;
        }
        break;
    case GDK_TOUCH_END:
    case GDK_TOUCH_CANCEL:
        lst = touch_list;
        while(lst)
        {
            data = (TouchData*)lst->data;
            if(data->seq == event->sequence)
            {
                touch_list = g_list_remove(touch_list, data);
                g_free(data);
                break;
            }
            lst = lst->next;
        }
        break;
    default:
        printf("Unknown touch type\n");
    }
    return FALSE;
}

gboolean on_draw(GtkWidget *widget, cairo_t *cr)
{
    TouchData *data;
    cairo_set_line_width(icr, 3);
    GList *lst = touch_list;
    while(lst)
    {
        data = (TouchData*)lst->data;
        cairo_set_source_rgb(icr, data->r, data->g, data->b);
        cairo_move_to(icr, data->old_x * 500, data->old_y * 500);
        cairo_line_to(icr, data->x * 500, data->y * 500);
        cairo_stroke(icr);
        lst = lst->next;
    }
    cairo_set_source_surface(cr, image, 0, 0);
    cairo_paint(cr);
    return FALSE;
}

int main(int argc, char *argv[])
{
    GtkWidget *window;
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    image = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 500, 500);
    icr = cairo_create(image);
    cairo_set_source_rgb(icr, 1.0, 1.0, 1.0);
    cairo_paint(icr);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(window, "touch-event", G_CALLBACK(on_touch), NULL);
    g_signal_connect(window, "draw", G_CALLBACK(on_draw), NULL);
    gtk_widget_set_events(window, GDK_TOUCH_MASK);

    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_widget_set_app_paintable(window, TRUE);

    randseed = g_rand_new();

    gtk_widget_show(window);
    gtk_main();
    return 0;
}
