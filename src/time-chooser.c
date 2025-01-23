/* time-chooser.c
 *
 * Copyright 2024-2025 Kelvin Novais
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <glib/gi18n.h>

#include "database-connection/database-connection.h"

#include "time-chooser.h"

struct _TimeChooser
{
  GtkBox                parent_instance;

  // Widgets
  GtkSpinButton         *h_spinbutton;
  GtkSpinButton         *m_spinbutton;
  AdwBin                *am_pm_bin;
  GtkButton             *am_pm_button;

  // Instance variables
  Period                 period;
};

// Translations
static const
gchar *am_pm[] =
{
  // translators: "AM" period, on 12-hour format
  N_("AM"),
  // translators: "PM" period, on 12-hour format
  N_("PM")
};

G_DEFINE_FINAL_TYPE (TimeChooser, time_chooser, GTK_TYPE_BOX)

static void
time_chooser_set_am_pm_label (TimeChooser *self, Period period)
{
  self->period = period;
  gtk_button_set_label (self->am_pm_button, gettext (am_pm[period]));
}

void
time_chooser_set_hour24 (TimeChooser *self, gdouble hour24)
{
  guint8 hour = (guint8) hour24;
  Period period;

  if (time_converter_get_format () == TIME_FORMAT_TWELVE)
    {
      time_converter_to_twelve_format (hour24, &hour, &period);
      time_chooser_set_am_pm_label (self, period);
    }

  gtk_spin_button_set_value (self->h_spinbutton, hour);
}

void
time_chooser_set_minutes (TimeChooser *self, gdouble minutes)
{
  gtk_spin_button_set_value (self->m_spinbutton, minutes);
}

guint8
time_chooser_get_hour24 (TimeChooser *self)
{
  gdouble _hour = gtk_spin_button_get_value (self->h_spinbutton);
  guint8 hour = 0;

  if (time_converter_get_format () == TIME_FORMAT_TWELVE)
    time_converter_to_twentyfour_format (_hour, &hour, self->period);
  else
    hour = (guint8) _hour;

  return hour;
}

guint8
time_chooser_get_minutes (TimeChooser *self)
{
  return (uint8_t) gtk_spin_button_get_value (self->m_spinbutton);
}

static gboolean
time_chooser_show_leading_zeros (GtkSpinButton *spin,
                                 gpointer       data)
{
   gchar *text;
   gint value;

   value = gtk_spin_button_get_value_as_int (spin);
   text = g_strdup_printf ("%02d", value);
   gtk_editable_set_text (GTK_EDITABLE (spin), text);
   g_free (text);

   return TRUE;
}

static void
time_chooser_am_pm_invert_label (GtkButton *button,
                                 gpointer   user_data)
{
  TimeChooser *self = TIME_CHOOSER (user_data);
  time_chooser_set_am_pm_label (self, !self->period);
}

static void
time_chooser_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), TIME_TYPE_CHOOSER);

  G_OBJECT_CLASS (time_chooser_parent_class)->dispose (gobject);
}

static void
time_chooser_class_init (TimeChooserClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/time-chooser.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, TimeChooser, h_spinbutton);
  gtk_widget_class_bind_template_child (widget_class, TimeChooser, m_spinbutton);
  gtk_widget_class_bind_template_child (widget_class, TimeChooser, am_pm_bin);
  gtk_widget_class_bind_template_child (widget_class, TimeChooser, am_pm_button);

  G_OBJECT_CLASS (klass)->dispose = time_chooser_dispose;
}

static void
time_chooser_init (TimeChooser *self)
{
  g_autoptr (GDateTime) now = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->h_spinbutton,
                    "output",
                    G_CALLBACK (time_chooser_show_leading_zeros),
                    NULL);

  g_signal_connect (self->m_spinbutton,
                    "output",
                    G_CALLBACK (time_chooser_show_leading_zeros),
                    NULL);

  g_signal_connect (self->am_pm_button,
                    "clicked",
                    G_CALLBACK (time_chooser_am_pm_invert_label),
                    self);

  // Set current time*
  now = g_date_time_new_now_local ();

  if (time_converter_get_format () == TIME_FORMAT_TWELVE)
    {
      g_autofree gchar *formatted_time = NULL;

      // Limit the hour spinbutton to 12 hours format
      gtk_spin_button_set_range (self->h_spinbutton, 1, 12);

      // Make the button visible
      gtk_widget_set_visible (GTK_WIDGET (self->am_pm_bin), TRUE);

      // TimeChooser defaults to current time*, so set the proper AM/PM label
      formatted_time = g_date_time_format (now, "%I:%M %p");
      if (strstr (formatted_time, "AM"))
        time_chooser_set_am_pm_label (self, PERIOD_AM);
      else
        time_chooser_set_am_pm_label (self, PERIOD_PM);
    }
  else
    {
      // Limit the hour spinbutton to 24 hours
      gtk_spin_button_set_range (self->h_spinbutton, 0, 23);
    }

  time_chooser_set_hour24 (self, (gdouble) g_date_time_get_hour (now));
  time_chooser_set_minutes (self, (gdouble) g_date_time_get_minute (now));

  g_debug ("TimeChooser now [HH:MM]: %02d:%02d",
           g_date_time_get_hour (now),
           g_date_time_get_minute (now));
}

TimeChooser *
time_chooser_new (void)
{
  return TIME_CHOOSER (g_object_new (TIME_TYPE_CHOOSER, NULL));
}
