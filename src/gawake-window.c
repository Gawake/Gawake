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

#include "config.h"

#include "gawake-window.h"
#include "rule-face.h"

#define ALLOW_MANAGING_RULES
#define ALLOW_MANAGING_CONFIGURATION
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_CONFIGURATION
#undef ALLOW_MANAGING_RULES

struct _GawakeWindow
{
  AdwApplicationWindow     parent_instance;

  /* Template widgets */
  AdwBin                  *turn_on_page;
  AdwBin                  *turn_off_page;
  GtkButton               *add_button;
  AdwViewStack            *stack;
  RuleFace                *turn_on_page_face;
  RuleFace                *turn_off_page_face;

  /* Instance variables */
  gint                     database_connection_status;
};

G_DEFINE_FINAL_TYPE (GawakeWindow, gawake_window, ADW_TYPE_APPLICATION_WINDOW)

static Table
gawake_window_get_table_from_page (GawakeWindow *self)
{
  const gchar *page_name = NULL;
  Table table;

  page_name = adw_view_stack_get_visible_child_name (self->stack);

  table = (g_strcmp0 (page_name, "on")) ? TABLE_OFF : TABLE_ON;

  return table;
}

// Adding a rule
static void
gawake_window_add_button_clicked (GtkButton *self,
                                  gpointer   user_data)
{
  GawakeWindow *window = GAWAKE_WINDOW (user_data);

  if (gawake_window_get_table_from_page (window) == TABLE_ON)
    rule_face_open_setup_add_dialog (window->turn_on_page_face);
  else
    rule_face_open_setup_add_dialog (window->turn_off_page_face);
}

static void
gawake_window_class_init (GawakeWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/gawake-window.ui");

  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_on_page);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_off_page);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, add_button);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, stack);
}

static void
gawake_window_init (GawakeWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->add_button,
                    "clicked",
                    G_CALLBACK (gawake_window_add_button_clicked),
                    self);

  // Database
  self->database_connection_status = connect_database (false);

  /* if (self->database_connection_status == SQLITE_OK) */
  /*   { */
  /*   } */

  self->turn_on_page_face = rule_face_new (RULE_FACE_TYPE_TURN_ON);
  adw_bin_set_child (self->turn_on_page, GTK_WIDGET (self->turn_on_page_face));

  self->turn_off_page_face = rule_face_new (RULE_FACE_TYPE_TURN_OFF);
  adw_bin_set_child (self->turn_off_page, GTK_WIDGET (self->turn_off_page_face));
}
