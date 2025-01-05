/* rule-row.c
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

/* TODO
 * Translate modes
 *
 * Signal to delete rule
 */

#include <glib/gi18n.h>
#include <inttypes.h>

#include "rule-row.h"

struct _RuleRow
{
  GtkListBoxRow              parent_instance;

  /* Template widgets */
  GtkLabel                  *title;
  GtkLabel                  *time;
  GtkLabel                  *repeats;
  GtkLabel                  *mode;
  GtkRevealer               *mode_revealer;
  GtkSwitch                 *active_toggle;
  GtkButton                 *delete_button;

  /* Instance variables */
  gint                       rule_id;
  Table                      table;
};

static const
gchar *days_plural[] =
{
  N_("Sundays"),
  N_("Mondays"),
  N_("Tuesdays"),
  N_("Wednesdays"),
  N_("Thursdays"),
  N_("Fridays"),
  N_("Saturdays")
};

static const
gchar *terms_plural[] =
{
  N_("Weekdays"),
  N_("Weekends")
};

typedef enum
{
  TERMS_PLURAL_WEEKDAYS,
  TERMS_PLURAL_WEEKENDS,
  TERMS_PLURAL_LAST
} TermsPlural;

G_DEFINE_TYPE (RuleRow, rule_row, GTK_TYPE_LIST_BOX_ROW)

guint16
rule_row_get_id (RuleRow *self)
{
  g_debug ("Returning rule id %d", self->rule_id);
  return self->rule_id;
}

static void
rule_row_set_id (RuleRow *self,
                 guint16  id)
{
  self->rule_id = id;
}

static void
rule_row_set_title (RuleRow  *self,
                    gchar    *title)
{
  gtk_label_set_text (self->title, title);
}

static void
rule_row_set_time (RuleRow  *self,
                   guint8    hour,
                   guint8    minutes)
{
  // HH:MM AM'\n' == 9
  const gulong time_length = 6;
  gchar rule_time[time_length];

  g_snprintf (rule_time, time_length,
              "%02d:%02d", hour, minutes);

  gtk_label_set_text (self->time, rule_time);
}

static void
rule_row_set_mode (RuleRow  *self,
                   Mode      mode)
{
  g_autofree gchar *formatted_mode = NULL;

  // translators: the rule mode
  formatted_mode = g_strconcat (_("Mode: "), MODE[mode], NULL);

  gtk_revealer_set_reveal_child (self->mode_revealer, TRUE);
  gtk_label_set_text (self->mode, formatted_mode);
}

static void
rule_row_set_table (RuleRow  *self,
                    Table     table)
{
  self->table = table;
}

// Bruh, it's 01/01/2025, 00:24 and I'm writing this code
static void
rule_row_set_repeats (RuleRow    *self,
                      gboolean    days[7])
{
  gint sum = 0;
  GString *repeated_days = g_string_new ("");
  g_autofree gchar *repeated_days_formatted = NULL;

  for (gint i = 0; i < 7; i++)
    sum += (days[i]) ? 1 : 0;

  // Set a string for repetition
  switch (sum)
    {
    case 7:
      // ...if all days are set;
      gtk_label_set_text (self->repeats, _("Every Day"));
      break;

    case 5:
      // ...weekdays
      if (!days[0] && !days[6])
        {
          gtk_label_set_text (self->repeats, gettext (terms_plural[TERMS_PLURAL_WEEKDAYS]));
          break;
        }

    case 2:
      // ...weekends
      if (days[0] && days[6])
        {
          gtk_label_set_text (self->repeats, gettext (terms_plural[TERMS_PLURAL_WEEKENDS]));
          break;
        }

    case 1:
      // ...if only one day is set
      for (int i = 0; i < 7; i++) {
        if (days[i])
          {
            gtk_label_set_text (self->repeats, gettext (days_plural[i]));
            break;
          }
      }
      break;

    case 0:
      // ...if any day is set;
      // translators: the rule repeats any day of the week
      gtk_label_set_text (self->repeats, _("Any Day"));
      break;

    default:
      // ...if only some days are set
      for (gint i = 0; i < 7; i++)
        if (days[i])
          g_string_append_printf (repeated_days,
                                  "%s, ",
                                  gettext (days_plural[i]));

      // remove the last unnecessary ", "
      repeated_days = g_string_truncate (repeated_days, repeated_days->len - 2);

      repeated_days_formatted = g_string_free_and_steal (repeated_days);
      gtk_label_set_text (self->repeats, repeated_days_formatted);
      repeated_days = NULL;
      break;
  }

  if (repeated_days != NULL)
    g_string_free (repeated_days, TRUE);
}

static void
rule_row_set_active (RuleRow      *self,
                     gboolean      active)
{
  gtk_switch_set_active (self->active_toggle, active);
}

static gboolean
rule_row_change_active (GtkSwitch* self,
                        gboolean state,
                        gpointer user_data)
{
  gint status = 0;
  RuleRow *row = RULE_ROW (user_data);

  status = rule_enable_disable (row->rule_id, row->table, (bool) state);

  if (status != EXIT_FAILURE)
    gtk_switch_set_state (self, state);

  return TRUE;
}

static void
rule_row_delete_rule (GtkButton *self,
                      gpointer   user_data)
{
  gint status = 0;
  RuleRow *row = RULE_ROW (user_data);

  status = rule_delete (row->rule_id, row->table);

  if (status == EXIT_SUCCESS)
    gtk_widget_set_visible (GTK_WIDGET (row), FALSE);

  // TODO emit warning on failure
  // delete widget
}

void
rule_row_set_fields (RuleRow *self,
                     Rule     rule)
{
  rule_row_set_id (self, rule.id);
  rule_row_set_title (self, rule.name);
  rule_row_set_time (self, rule.hour, rule.minutes);

  if (rule.table == TABLE_OFF)
    rule_row_set_mode (self, rule.mode);

  rule_row_set_table (self, rule.table);
  rule_row_set_repeats (self, (gboolean *) rule.days);
  rule_row_set_active (self, (gboolean) rule.active);
}

static void
rule_row_class_init (RuleRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/rule-row.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, RuleRow, title);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, time);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, repeats);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, mode_revealer);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, mode);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, active_toggle);
  gtk_widget_class_bind_template_child (widget_class, RuleRow, delete_button);
}

static void
rule_row_init (RuleRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_label_set_text (self->title, _("Unnamed rule"));
  gtk_label_set_text (self->time, "--:--");

  // Signals
  g_signal_connect (self->active_toggle,
                    "state-set",
                    G_CALLBACK (rule_row_change_active),
                    self);

  g_signal_connect (self->delete_button,
                    "clicked",
                    G_CALLBACK (rule_row_delete_rule),
                    self);
}

RuleRow *
rule_row_new (void)
{
  return RULE_ROW (g_object_new (RULE_TYPE_ROW, NULL));
}
