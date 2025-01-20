/* custom-schedule-face.c
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

#include "custom-schedule-face.h"

#define ALLOW_MANAGING_RULES
#include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

#include "time-chooser.h"
#include "mode-row.h"

struct _CustomScheduleFace
{
  AdwBin               parent_instance;

  // Widgets
  AdwToastOverlay     *toast_overlay;
  TimeChooser         *time_chooser;
  GtkCalendar         *calendar;
  GtkButton           *action_button;
  ModeRow             *mode_row;
};

G_DEFINE_FINAL_TYPE (CustomScheduleFace, custom_schedule_face, ADW_TYPE_BIN)

static void
custom_schedule_face_action_button_clicked (GtkButton *button,
                                            gpointer   user_data)
{
  guint8 hour, minutes;
  guint8 day, month;
  guint16 year;
  guint8 mode;
  GDateTime *datetime = NULL; // TODO free
  CustomScheduleFace *self = CUSTOM_SCHEDULE_FACE (user_data);

  // Time
  hour = time_chooser_get_hour (self->time_chooser);
  minutes = time_chooser_get_minutes (self->time_chooser);

  // Date
  datetime = gtk_calendar_get_date (self->calendar);
  day = (guint8) g_date_time_get_day_of_month (datetime);
  month = (guint8) g_date_time_get_month (datetime);
  year = (guint16) g_date_time_get_year (datetime);

  // Mode
  mode = (guint8) mode_row_get_mode (self->mode_row);

  g_debug ("CustomScheduleFace values:\n"\
           "\tHH:MM - %02d:%02d\n"\
           "\tDD/MM/YYYY - %02d/%02d/%d\n"\
           "\tMode: %d",
           hour, minutes,
           day, month, year,
           mode);

  if (rule_custom_schedule (hour, minutes, day, month, year, mode) == EXIT_SUCCESS)
    {
      // TODO countdown
    }
  else
    adw_toast_overlay_add_toast (self->toast_overlay, adw_toast_new (_("Invalid values")));

  g_date_time_unref (datetime);
}

static void
custom_schedule_face_class_init (CustomScheduleFaceClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/custom-schedule-face.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, CustomScheduleFace, time_chooser);
  gtk_widget_class_bind_template_child (widget_class, CustomScheduleFace, calendar);
  gtk_widget_class_bind_template_child (widget_class, CustomScheduleFace, action_button);
  gtk_widget_class_bind_template_child (widget_class, CustomScheduleFace, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, CustomScheduleFace, mode_row);
}

static void
custom_schedule_face_init (CustomScheduleFace *self)
{
  Mode default_mode;

  // Ensure types of custom widgets
  g_type_ensure (TIME_TYPE_CHOOSER);
  g_type_ensure (MODE_TYPE_ROW);

  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->action_button,
                    "clicked",
                    G_CALLBACK (custom_schedule_face_action_button_clicked),
                    self);

  // TODO connect database before initializing template
  if (configuration_get_default_mode (&default_mode) == EXIT_SUCCESS)
    mode_row_set_mode (self->mode_row, default_mode);
  else
    {} // g_warning ("Failed to load default mode to CustomScheduleFace");
}

CustomScheduleFace *
custom_schedule_face_new (void)
{
  return CUSTOM_SCHEDULE_FACE (g_object_new (CUSTOM_TYPE_SCHEDULE_FACE, NULL));
}
