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

#include "time-chooser.h"

struct _TimeChooser
{
  GtkBox                parent_instance;

  GtkSpinButton         *h_spinbutton;
  GtkSpinButton         *m_spinbutton;
};

G_DEFINE_FINAL_TYPE (TimeChooser, time_chooser, GTK_TYPE_BOX)

void
time_chooser_set_hour (TimeChooser *self, gdouble hour)
{
  gtk_spin_button_set_value (self->h_spinbutton, hour);
}

void
time_chooser_set_minutes (TimeChooser *self, gdouble minutes)
{
  gtk_spin_button_set_value (self->m_spinbutton, minutes);
}

guint8
time_chooser_get_hour (TimeChooser *self)
{
  return (uint8_t) gtk_spin_button_get_value (self->h_spinbutton);
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

  G_OBJECT_CLASS (klass)->dispose = time_chooser_dispose;

}

static void
time_chooser_init (TimeChooser *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->h_spinbutton,
                    "output",
                    G_CALLBACK (time_chooser_show_leading_zeros),
                    NULL);

  g_signal_connect (self->m_spinbutton,
                    "output",
                    G_CALLBACK (time_chooser_show_leading_zeros),
                    NULL);
}

TimeChooser *
time_chooser_new (void)
{
  return TIME_CHOOSER (g_object_new (TIME_TYPE_CHOOSER, NULL));
}
