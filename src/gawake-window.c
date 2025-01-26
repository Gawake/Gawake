/* gawake-window.c
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

#include "config.h"

#include "gawake-window.h"
#include "custom-schedule-face.h"
#include "rule-face.h"
#include "error-dialog.h"

#define ALLOW_MANAGING_RULES
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

struct _GawakeWindow
{
  AdwApplicationWindow     parent_instance;

  // Template widgets
  AdwToastOverlay         *toast_overlay;
  AdwViewStack            *stack;
  AdwBin                  *turn_on_page;
  AdwBin                  *turn_off_page;
  RuleFace                *turn_on_page_face;
  RuleFace                *turn_off_page_face;

  GtkStack                *action_button_stack;
  GtkButton               *add_button;
  GtkButton               *direct_schedule_button;

  // Instance variables
  GtkWindow               *error_dialog;
  ErrorDialogType          error_dialog_type;
  gint                     database_connection_status;
};

G_DEFINE_FINAL_TYPE (GawakeWindow, gawake_window, ADW_TYPE_APPLICATION_WINDOW)

static Table
gawake_window_get_table_from_page (GawakeWindow *self)
{
  const gchar *page_name = NULL;
  Table table = TABLE_LAST; // invalid

  page_name = adw_view_stack_get_visible_child_name (self->stack);

  if (g_strcmp0 (page_name, "on") == 0)
    table = TABLE_ON;

  if (g_strcmp0 (page_name, "off") == 0)
    table = TABLE_OFF;

  return table;
}

// Adding a rule
static void
gawake_window_add_button_clicked (GtkButton *button,
                                  gpointer   user_data)
{
  GawakeWindow *self = GAWAKE_WINDOW (user_data);
  Table table = gawake_window_get_table_from_page (self);

  switch (table)
    {
    case TABLE_ON:
      rule_face_open_setup_add_dialog (self->turn_on_page_face);
      break;

    case TABLE_OFF:
      rule_face_open_setup_add_dialog (self->turn_off_page_face);
      break;

    case TABLE_LAST:
    default:
      g_warning ("Invalid table from ViewStack");
      return;
    }
}

static void
gawake_window_direct_schedule_button_clicked (GtkButton *button,
                                              gpointer   user_data)
{
  GawakeWindow *self = GAWAKE_WINDOW (user_data);
  RtcwakeArgs rtcwake_args;
  RtcwakeArgsReturn ret;

  ret = rule_get_upcoming_on (&rtcwake_args, MODE_LAST);

  switch (ret)
    {
    case RTCWAKE_ARGS_RETURN_SUCESS:
      // TODO send DBus signal
      g_message ("Running direct schedule");
      break;

    case RTCWAKE_ARGS_RETURN_NOT_FOUND:
      adw_toast_overlay_add_toast (self->toast_overlay,
                                   adw_toast_new (_("Any turn on rule found. please create one first")));
      break;

    case RTCWAKE_ARGS_RETURN_INVALID:
      adw_toast_overlay_add_toast (self->toast_overlay,
                                   adw_toast_new (_("Failed: invalid arguments")));
      break;

    case RTCWAKE_ARGS_RETURN_FAILURE:
    default:
      adw_toast_overlay_add_toast (self->toast_overlay,
                                   adw_toast_new (_("Failed to run direct schedule")));
    }
}

static gboolean
gawake_window_on_error_dialog_close_request (GtkWindow *window,
                                             gpointer   user_data)
{
  GawakeWindow *self = GAWAKE_WINDOW (user_data);

  gtk_window_close (GTK_WINDOW (self));

  return FALSE;
}

static void
gawake_window_show_error_dialog (gpointer user_data)
{
  GawakeWindow *self = GAWAKE_WINDOW (user_data);

  self->error_dialog = GTK_WINDOW (error_dialog_new (self->error_dialog_type));

  g_signal_connect (self->error_dialog,
                    "close-request",
                    G_CALLBACK (gawake_window_on_error_dialog_close_request),
                    self);

  gtk_window_set_transient_for (self->error_dialog, GTK_WINDOW (self));
  gtk_window_present (self->error_dialog);
}

static void
gawake_window_face_changed (GObject    *object,
                            GParamSpec *pspec,
                            gpointer    user_data)
{
  const gchar *page_name = NULL;
  GawakeWindow *self = GAWAKE_WINDOW (user_data);

  page_name = adw_view_stack_get_visible_child_name (self->stack);

  if (g_strcmp0 (page_name, "custom-schedule") == 0)
    gtk_stack_set_visible_child (self->action_button_stack,
                                 GTK_WIDGET (self->direct_schedule_button));
  else
    gtk_stack_set_visible_child (self->action_button_stack,
                                 GTK_WIDGET (self->add_button));
}

static void
gawake_window_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), GAWAKE_TYPE_WINDOW);

  G_OBJECT_CLASS (gawake_window_parent_class)->dispose (gobject);
}

static void
gawake_window_class_init (GawakeWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/gawake-window.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_on_page);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_off_page);

  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, action_button_stack);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, add_button);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, direct_schedule_button);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, toast_overlay);

  G_OBJECT_CLASS (klass)->dispose = gawake_window_dispose;
}

static void
gawake_window_init (GawakeWindow *self)
{
  self->error_dialog = NULL;

  // Ensure the type of my custom widgets
  g_type_ensure (CUSTOM_TYPE_SCHEDULE_FACE);

  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->add_button,
                    "clicked",
                    G_CALLBACK (gawake_window_add_button_clicked),
                    self);

  g_signal_connect (self->direct_schedule_button,
                    "clicked",
                    G_CALLBACK (gawake_window_direct_schedule_button_clicked),
                    self);

  g_signal_connect (self->stack,
                    "notify::visible-child",
                    G_CALLBACK (gawake_window_face_changed),
                    self);

#if !FLATPAK
  // Check user group
  if (check_user_group ())
    {
      self->error_dialog_type = ERROR_DIALOG_TYPE_USER_GROUP_ERROR;
      g_timeout_add_once (100, gawake_window_show_error_dialog, self);
      return;
    }
#endif

  // Database connection
  self->database_connection_status = connect_database (false);

  if (self->database_connection_status == SQLITE_OK)
    {
      self->turn_on_page_face = rule_face_new (RULE_FACE_TYPE_TURN_ON);
      adw_bin_set_child (self->turn_on_page, GTK_WIDGET (self->turn_on_page_face));

      self->turn_off_page_face = rule_face_new (RULE_FACE_TYPE_TURN_OFF);
      adw_bin_set_child (self->turn_off_page, GTK_WIDGET (self->turn_off_page_face));
    }
  else
    {
      self->error_dialog_type = ERROR_DIALOG_TYPE_DATABASE_ERROR;
      g_timeout_add_once (100, gawake_window_show_error_dialog, self);
    }
}
