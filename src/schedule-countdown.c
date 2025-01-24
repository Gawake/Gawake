/* schedule-countdown.c
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

#define COUNTDOWN_START 5

#include <glib/gi18n.h>

#include "schedule-countdown.h"

struct _ScheduleCountdown
{
  AdwWindow              parent_instance;

  // Widgets
  AdwStatusPage         *status_page;
  GtkButton             *schedule_button;
  GtkButton             *cancel_button;

  // Instance variables
  gint                   seconds_remaining;
  GSource               *timer_source;
  gboolean               finalized;
};

// Signals
enum
{
  SIGNAL_SCHEDULE,

  N_SIGNALS
};

static guint obj_signals[N_SIGNALS];

G_DEFINE_FINAL_TYPE (ScheduleCountdown, schedule_countdown, ADW_TYPE_WINDOW)

void
schedule_countdown_finalize (ScheduleCountdown *self)
{
  g_source_destroy (self->timer_source);

  gtk_window_close (GTK_WINDOW (self));
}

static void
schedule_countdown_cancel_button_clicked (GtkButton *button,
                                          gpointer   user_data)
{
  ScheduleCountdown *self = SCHEDULE_COUNTDOWN (user_data);

  g_signal_emit (self,
                 obj_signals[SIGNAL_SCHEDULE],
                 0,
                 FALSE);
}

static void
schedule_countdown_schedule_button_clicked (GtkButton *button,
                                            gpointer   user_data)
{
  ScheduleCountdown *self = SCHEDULE_COUNTDOWN (user_data);

  g_signal_emit (self,
                 obj_signals[SIGNAL_SCHEDULE],
                 0,
                 TRUE);
}

static void
schedule_countdown_set_title (ScheduleCountdown *self,
                              gint              seconds)
{
  // "MM:SS\0" == 6
  static gchar time_remaining[6];

  g_snprintf (time_remaining, 6, "00:%02d", seconds);

  adw_status_page_set_title (self->status_page, time_remaining);
}

static gboolean
schedule_countdown_seconds_counter (gpointer user_data)
{
  ScheduleCountdown *self = SCHEDULE_COUNTDOWN (user_data);

  if (self->seconds_remaining <= 0)
    {
      schedule_countdown_set_title (self, 0);

      g_signal_emit (self,
                     obj_signals[SIGNAL_SCHEDULE],
                     0,
                     TRUE);

      return FALSE;
    }

  schedule_countdown_set_title (self,
                                self->seconds_remaining--);

  return TRUE;
}

static void
schedule_countdown_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), SCHEDULE_TYPE_COUNTDOWN);

  G_OBJECT_CLASS (schedule_countdown_parent_class)->dispose (gobject);
}

static void
schedule_countdown_class_init (ScheduleCountdownClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/schedule-countdown.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, ScheduleCountdown, status_page);
  gtk_widget_class_bind_template_child (widget_class, ScheduleCountdown, schedule_button);
  gtk_widget_class_bind_template_child (widget_class, ScheduleCountdown, cancel_button);

  // Signals
  obj_signals[SIGNAL_SCHEDULE] =
    g_signal_new ("schedule",
                  SCHEDULE_TYPE_COUNTDOWN,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,            // no return value
                  1,                      // 1 argument
                  G_TYPE_BOOLEAN);        // scheduled

  G_OBJECT_CLASS (klass)->dispose = schedule_countdown_dispose;
}

static void
schedule_countdown_init (ScheduleCountdown *self)
{

  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->schedule_button,
                    "clicked",
                    G_CALLBACK (schedule_countdown_schedule_button_clicked),
                    self);

  g_signal_connect (self->cancel_button,
                    "clicked",
                    G_CALLBACK (schedule_countdown_cancel_button_clicked),
                    self);

  // Set the title...
  schedule_countdown_set_title (self, COUNTDOWN_START);

  // ...and start countdown
  // Note: the first second is counted at 'g_source_set_callback' call, so subtract 1
  self->seconds_remaining = COUNTDOWN_START - 1;
  self->timer_source = g_timeout_source_new (1000);
  g_source_attach (self->timer_source, NULL);
  g_source_set_callback (self->timer_source,
                         schedule_countdown_seconds_counter,
                         self,
                         NULL);
}

ScheduleCountdown *
schedule_countdown_new (void)
{
  return SCHEDULE_COUNTDOWN (g_object_new (SCHEDULE_TYPE_COUNTDOWN, NULL));
}
