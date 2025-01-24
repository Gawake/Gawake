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
#include "schedule-countdown.h"

struct _CustomScheduleFace
{
  AdwBin               parent_instance;

  // Widgets
  AdwToastOverlay     *toast_overlay;
  TimeChooser         *time_chooser;
  GtkCalendar         *calendar;
  GtkButton           *action_button;
  ModeRow             *mode_row;

  // Instance variables
  RtcwakeArgs          rtcwake_args;
};

G_DEFINE_FINAL_TYPE (CustomScheduleFace, custom_schedule_face, ADW_TYPE_BIN)

static void
custom_schedule_face_countdown_finished (ScheduleCountdown *countdown,
                                         gboolean           schedule,
                                         gpointer           user_data)
{
  CustomScheduleFace *self = CUSTOM_SCHEDULE_FACE (user_data);

  // Schedule if not canceled
  if (schedule)
    if (rule_custom_schedule (&self->rtcwake_args) == EXIT_FAILURE)
      adw_toast_overlay_add_toast (self->toast_overlay,
                                   adw_toast_new (_("Failed to make a custom schedule")));

  schedule_countdown_finalize (countdown);
}

static void
custom_schedule_face_action_button_clicked (GtkButton *button,
                                            gpointer   user_data)
{
  GDateTime *datetime = NULL;
  CustomScheduleFace *self = CUSTOM_SCHEDULE_FACE (user_data);

  // Time
  self->rtcwake_args.hour = time_chooser_get_hour24 (self->time_chooser);
  self->rtcwake_args.minutes = time_chooser_get_minutes (self->time_chooser);

  // Date
  datetime = gtk_calendar_get_date (self->calendar);
  self->rtcwake_args.day = (guint8) g_date_time_get_day_of_month (datetime);
  self->rtcwake_args.month = (guint8) g_date_time_get_month (datetime);
  self->rtcwake_args.year = (guint16) g_date_time_get_year (datetime);

  // Mode
  self->rtcwake_args.mode = (Mode) mode_row_get_mode (self->mode_row);

  g_debug ("CustomScheduleFace values:\n"\
           "\tHH:MM - %02d:%02d\n"\
           "\tDD/MM/YYYY - %02d/%02d/%d\n"\
           "\tMode: %d",
           self->rtcwake_args.hour, self->rtcwake_args.minutes,
           self->rtcwake_args.day, self->rtcwake_args.month, self->rtcwake_args.year,
           self->rtcwake_args.mode);

  if (rule_validade_rtcwake_args (&self->rtcwake_args) == EXIT_SUCCESS)
    {
      ScheduleCountdown *countdown = NULL;
      GtkWidget *parent = GTK_WIDGET (self);

      countdown = schedule_countdown_new ();

      g_signal_connect (countdown,
                        "schedule",
                        G_CALLBACK (custom_schedule_face_countdown_finished),
                        self);

      while ((parent = gtk_widget_get_parent (parent)) != NULL)
        {
          g_debug ("Parent widget: %s", G_OBJECT_TYPE_NAME (parent));
          if (GTK_IS_WINDOW (parent))
            {
              gtk_window_set_transient_for (GTK_WINDOW (countdown), GTK_WINDOW (parent));
              break;
            }
        }

      gtk_window_present (GTK_WINDOW (countdown));
    }
  else
    {
      adw_toast_overlay_add_toast (self->toast_overlay, adw_toast_new (_("Invalid values")));
    }

  g_date_time_unref (datetime);
}

static void
custom_schedule_face_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), CUSTOM_TYPE_SCHEDULE_FACE);

  G_OBJECT_CLASS (custom_schedule_face_parent_class)->dispose (gobject);
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

  G_OBJECT_CLASS (klass)->dispose = custom_schedule_face_dispose;
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
