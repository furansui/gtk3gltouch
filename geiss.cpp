//http://people.canonical.com/~stephenwebb/geis-v2-api/geis2_8c-example.html
//g++ geiss.cpp -o geiss.xps0 -fpermissive -lgeis

#include <errno.h>
#include <geis/geis.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>


void print_attr(GeisAttr attr)
{
  GeisString attr_name = geis_attr_name(attr);
  switch (geis_attr_type(attr))
  {
    case GEIS_ATTR_TYPE_BOOLEAN:
      printf("  \"%s\": %s\n", attr_name,
             geis_attr_value_to_boolean(attr) ? "true" : "false");
      break;
    case GEIS_ATTR_TYPE_FLOAT:
      printf("  \"%s\": %g\n", attr_name, geis_attr_value_to_float(attr));
      break;
    case GEIS_ATTR_TYPE_INTEGER:
      printf("  \"%s\": %d\n", attr_name, geis_attr_value_to_integer(attr));
      break;
    case GEIS_ATTR_TYPE_STRING:
      printf("  \"%s\": %s\n", attr_name, geis_attr_value_to_string(attr));
      break;
    default:
      break;
  }
}


void
dump_device_event(GeisEvent event)
{
  GeisDevice device;
  GeisAttr attr;
  GeisSize i;
  GeisInputDeviceId device_id = 0;

  attr = geis_event_attr_by_name(event, GEIS_EVENT_ATTRIBUTE_DEVICE);
  device = geis_attr_value_to_pointer(attr);
  printf("device %02d \"%s\"\n",
         geis_device_id(device), geis_device_name(device));
  for (i = 0; i < geis_device_attr_count(device); ++i)
  {
    print_attr(geis_device_attr(device, i));
  }
}


void
dump_gesture_event(GeisEvent event)
{
  GeisSize i;
  GeisTouchSet touchset;
  GeisGroupSet groupset;
  GeisAttr     attr;

  attr = geis_event_attr_by_name(event, GEIS_EVENT_ATTRIBUTE_TOUCHSET);
  touchset = geis_attr_value_to_pointer(attr);

  attr = geis_event_attr_by_name(event, GEIS_EVENT_ATTRIBUTE_GROUPSET);
  groupset = geis_attr_value_to_pointer(attr);

  printf("gesture\n");
  for (i= 0; i < geis_groupset_group_count(groupset); ++i)
  {
    GeisSize j;
    GeisGroup group = geis_groupset_group(groupset, i);
    printf("+group %u\n", geis_group_id(group));

    for (j=0; j < geis_group_frame_count(group); ++j)
    {
      GeisSize k;
      GeisFrame frame = geis_group_frame(group, j);
      GeisSize attr_count = geis_frame_attr_count(frame);

      printf("+frame %u\n", geis_frame_id(frame));
      for (k = 0; k < attr_count; ++k)
      {
        print_attr(geis_frame_attr(frame, k));
      }

      for (k = 0; k < geis_frame_touchid_count(frame); ++k)
      {
        GeisSize  touchid = geis_frame_touchid(frame, k);
        GeisTouch touch = geis_touchset_touch_by_id(touchset, touchid);
        GeisSize  n;
        printf("+touch %lu\n", k);
        for (n = 0; n < geis_touch_attr_count(touch); ++n)
        {
          print_attr(geis_touch_attr(touch, n));
        }
      }
    }
  }
}


int
main(int argc, char* argv[])
{
  GeisStatus status;
  Geis geis;
  GeisSubscription subscription;
  GeisFilter filter;
  int        fd;

  geis = geis_new(GEIS_INIT_TRACK_DEVICES,
                  NULL);
  subscription = geis_subscription_new(geis, "example", GEIS_SUBSCRIPTION_CONT);
  filter = geis_filter_new(geis, "filter");

  geis_filter_add_term(filter,
                       GEIS_FILTER_CLASS,
                       GEIS_GESTURE_ATTRIBUTE_TOUCHES, GEIS_FILTER_OP_EQ, 2,
                       NULL);

  status = geis_subscription_add_filter(subscription, filter);
  status = geis_subscription_activate(subscription);

  geis_get_configuration(geis, GEIS_CONFIGURATION_FD, &fd);

  while(1)
  {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);
    FD_SET(fd, &read_fds);
    int sstat = select(fd+1, &read_fds, NULL, NULL, NULL);
    if (sstat < 0)
    {
      fprintf(stderr, "error %d in select(): %s\n", errno, strerror(errno));
      break;
    }

    if (FD_ISSET(fd, &read_fds))
    {
      GeisEvent event;
      status = geis_dispatch_events(geis);
      status = geis_next_event(geis, &event);
      while (status == GEIS_STATUS_CONTINUE || status == GEIS_STATUS_SUCCESS)
      {
        switch (geis_event_type(event))
        {
          case GEIS_EVENT_DEVICE_AVAILABLE:
          case GEIS_EVENT_DEVICE_UNAVAILABLE:
            dump_device_event(event);
            break;

          case GEIS_EVENT_GESTURE_BEGIN:
          case GEIS_EVENT_GESTURE_UPDATE:
          case GEIS_EVENT_GESTURE_END:
            dump_gesture_event(event);
            break;
        }
        geis_event_delete(event);
        status = geis_next_event(geis, &event);
      }
    }

    if (FD_ISSET(0, &read_fds))
    {
      break;
    }
  }

  geis_subscription_delete(subscription);
  geis_delete(geis);
}
