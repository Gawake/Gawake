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

#include "rule-row.h"

#define ALLOW_MANAGING_RULES
#define ALLOW_MANAGING_CONFIGURATION
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_CONFIGURATION
#undef ALLOW_MANAGING_RULES

struct _GawakeWindow
{
  AdwApplicationWindow     parent_instance;

  /* Template widgets */
  GtkListBox              *turn_on_rules_listbox;
  GtkListBox              *turn_off_rules_listbox;

  /* Instance variables */
  gint                   database_connection_status;
};

G_DEFINE_FINAL_TYPE (GawakeWindow, gawake_window, ADW_TYPE_APPLICATION_WINDOW)

static void
gawake_window_populate_rules (GawakeWindow *self,
                              Table         table)
{
  Rule *rules = NULL;
  guint16 row_count = 0;

  if (rule_get_all (table, &rules, &row_count) == EXIT_FAILURE)
    return; // TODO emmit warning, set error to window

  for (guint16 row_idx = 0; row_idx < row_count; row_idx++)
    {
      RuleRow *row = rule_row_new ();
      rule_row_set_fields (row, rules[row_idx]);
      if (table == TABLE_ON)
        gtk_list_box_append (self->turn_on_rules_listbox, GTK_WIDGET (row));
      else
        gtk_list_box_append (self->turn_off_rules_listbox, GTK_WIDGET (row));
    }

  free (rules);
}

static void
gawake_window_class_init (GawakeWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/gawake-window.ui");

  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_on_rules_listbox);
  gtk_widget_class_bind_template_child (widget_class, GawakeWindow, turn_off_rules_listbox);
}

static void
gawake_window_init (GawakeWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->database_connection_status = connect_database (false);

  if (self->database_connection_status == SQLITE_OK)
    {
      gawake_window_populate_rules (self, TABLE_ON);
      gawake_window_populate_rules (self, TABLE_OFF);
    }
}
