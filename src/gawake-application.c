/* gawake-application.c
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

#include "config.h"
#include <glib/gi18n.h>

#include "gawake-application.h"
#include "gawake-window.h"
#include "gawake-preferences.h"

struct _GawakeApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_FINAL_TYPE (GawakeApplication, gawake_application, ADW_TYPE_APPLICATION)

GawakeApplication *
gawake_application_new (const char        *application_id,
                        GApplicationFlags  flags)
{
  g_return_val_if_fail (application_id != NULL, NULL);

  return g_object_new (GAWAKE_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}

static void
gawake_application_preferences_action (GSimpleAction *action,
                                       GVariant      *parameter,
                                       gpointer       app)
{
  GawakePreferences *preferences;
  GtkWindow *window;

  window = gtk_application_get_active_window (GTK_APPLICATION (app));
  preferences = gawake_preferences_new ();
  gtk_window_set_transient_for (GTK_WINDOW (preferences), GTK_WINDOW (window));
  gtk_window_present (GTK_WINDOW (preferences));
}

static void
gawake_application_activate (GApplication *app)
{
  GtkWindow *window;

  g_assert (GAWAKE_IS_APPLICATION (app));

  window = gtk_application_get_active_window (GTK_APPLICATION (app));

  if (window == NULL)
    window = g_object_new (GAWAKE_TYPE_WINDOW,
                           "application", app,
                           NULL);

  gtk_window_present (window);
}

static void
gawake_application_class_init (GawakeApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = gawake_application_activate;
}

static void
gawake_application_about_action (GSimpleAction *action,
                                 GVariant      *parameter,
                                 gpointer       user_data)
{
  static const char *developers[] = {"Kelvin Novais", NULL};
  GawakeApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (GAWAKE_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  adw_show_about_window (GTK_WINDOW (window),
                         "application-name", "Gawake",
                         "application-icon", "io.github.gawake.Gawake",
                         "developer-name", "Kelvin Novais",
                         // TODO
                         /* "comments", _("A Linux software to make your PC wake up on a scheduled time." */
                         /*               "\n\nIf you liked the app ❤️, consider giving it a star ⭐:"), */
                         /* "issue-url", "https://github.com/Gawake/Gawake/issues", */
                         /* "website", "https://github.com/Gawake/Gawake", */
                         // Translators: Replace "translator-credits" with your names, one name per line
                         "translator-credits", _("translator-credits"),
                         "version", "0.1.0",
                         "developers", developers,
                         "copyright", "© 2024 Kelvin Novais",
                         "license-type", GTK_LICENSE_GPL_3_0,
                         NULL);
}

static void
gawake_application_quit_action (GSimpleAction *action,
                                GVariant      *parameter,
                                gpointer       user_data)
{
  GawakeApplication *self = user_data;

  g_assert (GAWAKE_IS_APPLICATION (self));

  g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
  { "quit", gawake_application_quit_action },
  { "about", gawake_application_about_action },
  { "preferences", gawake_application_preferences_action }
};

static void
gawake_application_init (GawakeApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   app_actions,
                                   G_N_ELEMENTS (app_actions),
                                   self);
  gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                         "app.quit",
                                         (const char *[]) { "<primary>q", NULL });
}
