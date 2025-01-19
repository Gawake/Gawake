/* gawake-preferences.c
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

#define ALLOW_MANAGING_CONFIGURATION
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_CONFIGURATION

#include "mode-row.h"

#include "gawake-preferences.h"

struct _GawakePreferences
{
  AdwPreferencesWindow            parent_instance;

  // Widgets
  AdwActionRow                   *shutdown_action_row;
  ModeRow                        *mode_row;
  AdwActionRow                   *localtime_action_row;
  AdwActionRow                   *notification_time_row;
  GtkSpinButton                  *notification_spin_button;

  GtkWidget                      *shutdown_switch;
  GtkWidget                      *localtime_switch;
};

G_DEFINE_TYPE (GawakePreferences, gawake_preferences, ADW_TYPE_PREFERENCES_WINDOW)

static gboolean
gawake_preferences_switch_state_set (GtkSwitch *switch_button,
                                     gboolean   state,
                                     gpointer   user_data)
{
  GawakePreferences *self = GAWAKE_PREFERENCES (user_data);
  gint status = EXIT_FAILURE;

  if (switch_button == GTK_SWITCH (self->shutdown_switch))
    status = configuration_set_shutdown_fail ((bool) state);
  else if (switch_button == GTK_SWITCH (self->localtime_switch))
    status = configuration_set_localtime ((bool) state);
  else
    g_warning ("Invalid swicth");

  if (status == EXIT_FAILURE)
    {
      adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                            adw_toast_new (_("Operation failed")));
      gtk_switch_set_active (switch_button, !status);
      return TRUE;
    }

  return FALSE;
}

static void
gawake_preferences_notification_changed (GtkSpinButton *spin_button,
                                         gpointer       user_data)
{
  gdouble new_value = 1;
  GawakePreferences *self = GAWAKE_PREFERENCES (user_data);

  new_value = gtk_spin_button_get_value (spin_button);

  if (configuration_set_notification_time ((int) new_value) == EXIT_FAILURE)
    adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                      adw_toast_new (_("Operation failed")));
}

static gboolean
gawake_preferences_on_close_request (GtkWindow *self,
                                     gpointer   user_data)
{
  Mode mode = mode_row_get_mode (GAWAKE_PREFERENCES (self)->mode_row);

  if (configuration_set_default_mode (mode) == EXIT_FAILURE)
    g_warning ("Failed to save default mode");
  else
    g_info ("Default mode saved successfully");

  return FALSE;
}

static void
gawake_preferences_class_init (GawakePreferencesClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/gawake-preferences.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, GawakePreferences, shutdown_action_row);
  gtk_widget_class_bind_template_child (widget_class, GawakePreferences, mode_row);
  gtk_widget_class_bind_template_child (widget_class, GawakePreferences, localtime_action_row);
  gtk_widget_class_bind_template_child (widget_class, GawakePreferences, notification_time_row);
}

static void
gawake_preferences_init (GawakePreferences *self)
{
  bool shutdown_fail = FALSE;
  bool use_localtime = FALSE;
  gint notification_time = 1;
  Mode default_mode = MODE_OFF;

  // Ensure my custom widgets types
  g_type_ensure (MODE_TYPE_ROW);

  gtk_widget_init_template (GTK_WIDGET (self));

  // SHUTDOWN ON FAILURE
  if (configuration_get_shutdown_fail (&shutdown_fail))
    {
      adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                        adw_toast_new (_("Failed to get configuration")));
      gtk_widget_set_sensitive (GTK_WIDGET (self->shutdown_switch), FALSE);
    }
  else
    {
      // Note [1]
      self->shutdown_switch = gtk_switch_new ();
      gtk_widget_set_valign (self->shutdown_switch, GTK_ALIGN_CENTER);
      gtk_switch_set_state (GTK_SWITCH (self->shutdown_switch), shutdown_fail);
      gtk_switch_set_active (GTK_SWITCH (self->shutdown_switch), shutdown_fail);
      adw_action_row_add_suffix (self->shutdown_action_row, self->shutdown_switch);
      adw_action_row_set_activatable_widget (self->shutdown_action_row, self->shutdown_switch);
      g_signal_connect (self->shutdown_switch,
                        "state-set",
                        G_CALLBACK (gawake_preferences_switch_state_set),
                        self);
    }

  // NOTIFICATION TIME
  if (configuration_get_notification_time (&notification_time))
    {
      adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                        adw_toast_new (_("Failed to get configuration")));
      gtk_widget_set_sensitive (GTK_WIDGET (self->notification_spin_button), FALSE);
    }
  else
    {
      self->notification_spin_button = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (1, 60, 1));
      gtk_spin_button_set_value (self->notification_spin_button, (gdouble) notification_time);
      gtk_widget_set_valign (GTK_WIDGET (self->notification_spin_button), GTK_ALIGN_CENTER);
      adw_action_row_add_suffix (self->notification_time_row, GTK_WIDGET (self->notification_spin_button));
      adw_action_row_set_activatable_widget (self->notification_time_row, GTK_WIDGET (self->notification_spin_button));
      g_signal_connect (self->notification_spin_button,
                        "value-changed",
                        G_CALLBACK (gawake_preferences_notification_changed),
                        self);
    }

  // DEFAULT MODE
  if (configuration_get_default_mode (&default_mode))
    {
      adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                        adw_toast_new (_("Failed to get configuration")));
      gtk_widget_set_sensitive (GTK_WIDGET (self->mode_row), FALSE);
    }
  else
    {
      mode_row_set_mode (self->mode_row, default_mode);
    }

  // USE LOCALTIME
  if (configuration_get_localtime (&use_localtime))
    {
      adw_preferences_window_add_toast (ADW_PREFERENCES_WINDOW (self),
                                        adw_toast_new (_("Failed to get configuration")));
      gtk_widget_set_sensitive (GTK_WIDGET (self->localtime_switch), FALSE);
    }
  else
    {
      // Note [1]
      self->localtime_switch = gtk_switch_new ();
      gtk_widget_set_valign (self->localtime_switch, GTK_ALIGN_CENTER);
      gtk_switch_set_state (GTK_SWITCH (self->localtime_switch), use_localtime);
      gtk_switch_set_active (GTK_SWITCH (self->localtime_switch), use_localtime);
      adw_action_row_add_suffix (self->localtime_action_row, self->localtime_switch);
      adw_action_row_set_activatable_widget (self->localtime_action_row, self->localtime_switch);
      g_signal_connect (self->localtime_switch,
                        "state-set",
                        G_CALLBACK (gawake_preferences_switch_state_set),
                        self);
    }

  g_signal_connect (self,
                    "close-request",
                    G_CALLBACK (gawake_preferences_on_close_request),
                    NULL);
}

GawakePreferences *
gawake_preferences_new (void)
{
  return GAWAKE_PREFERENCES (g_object_new (GAWAKE_TYPE_PREFERENCES, NULL));
}

/* Note [1]
 * if need to bind more GtkSwitch to a non GSettings, create a widget with this boilerplate code
 */
