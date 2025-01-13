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

// Translations
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

// Properties
enum
{
  PROP_ID = 1,
  PROP_TABLE,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

// Signals
enum
{
  SIGNAL_RULE_DELETED,
  SIGNAL_ERROR,

  N_SIGNALS
};

static guint obj_signals[N_SIGNALS];

G_DEFINE_TYPE (RuleRow, rule_row, GTK_TYPE_LIST_BOX_ROW)

guint16
rule_row_get_id (RuleRow *self)
{
  return self->rule_id;
}

static void
rule_row_emit_deleted (RuleRow *self)
{
  g_signal_emit (self,
                 obj_signals[SIGNAL_RULE_DELETED],
                 0,
                 (guint) self->table);
}

static void
rule_row_emit_error (RuleRow     *self,
                     const gchar *error)
{
  g_signal_emit (self,
                 obj_signals[SIGNAL_ERROR],
                 0,
                 error);
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
                      bool        days[7])
{
  gint sum = 0;
  GString *repeated_days = g_string_new ("");
  g_autofree gchar *repeated_days_formatted = NULL;

  for (gint i = 0; i < 7; i++)
    sum += (days[i]) ? 1 : 0;

  // Set a string for repetition
  if (sum == 7)
    {
      // ...if all days are set;
      gtk_label_set_text (self->repeats, _("Every Day"));
    }

  else if (sum == 5
           && (!days[0] && !days[6]))
    {
      // ...weekdays
      gtk_label_set_text (self->repeats, gettext (terms_plural[TERMS_PLURAL_WEEKDAYS]));
    }

  else if (sum == 2
           && (days[0] && days[6]))
    {
      // ...weekends
      gtk_label_set_text (self->repeats, gettext (terms_plural[TERMS_PLURAL_WEEKENDS]));
    }

  else if (sum == 1)
    {
      // ...if only one day is set
      for (int i = 0; i < 7; i++)
        {
          if (days[i])
            {
              gtk_label_set_text (self->repeats, gettext (days_plural[i]));
              break;
            }
        }
    }

  else if (sum == 0)
    {
      // ...if any day is set;
      // translators: the rule repeats any day of the week
      gtk_label_set_text (self->repeats, _("Any Day"));
    }

  else
    {
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

  if (status == EXIT_FAILURE)
    rule_row_emit_error (row, _("Failed to change rule state"));
  else
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

  if (status == EXIT_FAILURE)
    rule_row_emit_error (row, _("Failed to dele rule"));
  else
    gtk_widget_set_visible (GTK_WIDGET (row), FALSE);

  rule_row_emit_deleted (row);
}

void
rule_row_update_fields (RuleRow *self)
{
  Rule rule;

  if (rule_get_single (self->rule_id, self->table, &rule) == EXIT_FAILURE)
    {
      rule_row_emit_error (self, _("Failed to update rule"));
      return;
    }

  rule_row_set_id (self, rule.id);
  rule_row_set_title (self, rule.name);
  rule_row_set_time (self, rule.hour, rule.minutes);

  if (rule.table == TABLE_OFF)
    rule_row_set_mode (self, rule.mode);

  rule_row_set_table (self, rule.table);
  rule_row_set_repeats (self, rule.days);
  rule_row_set_active (self, (gboolean) rule.active);
}

static void
rule_row_constructed (GObject *gobject)
{
  RuleRow *self = RULE_ROW (gobject);
  Rule rule =
    {
      .id = 0,
      .name = "",
      .hour = 00,
      .minutes = 00,
      .days = {0, 0, 0, 0, 0, 0, 0},
      .mode = MODE_LAST,
      .table = TABLE_LAST
    };
  gint status;
  G_OBJECT_CLASS (rule_row_parent_class)->constructed (gobject);

  status = rule_get_single (self->rule_id, self->table, &rule);

  if (status == EXIT_SUCCESS)
    {
      rule_row_set_id (self, rule.id);
      rule_row_set_title (self, rule.name);
      rule_row_set_time (self, rule.hour, rule.minutes);

      if (rule.table == TABLE_OFF)
        rule_row_set_mode (self, rule.mode);

      rule_row_set_table (self, rule.table);
      rule_row_set_repeats (self, rule.days);
      rule_row_set_active (self, (gboolean) rule.active);
    }
  else
    {
      g_warning ("Failed to get rule at construct step");
    }
}

static void
rule_row_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  RuleRow *self = RULE_ROW (object);

  switch (property_id)
    {
    case PROP_ID:
      self->rule_id = (uint16_t) g_value_get_int (value);
      break;

    case PROP_TABLE:
      self->table = (Table) g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
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

  // Properties
  obj_properties[PROP_ID] =
    g_param_spec_int ("id",
                      NULL, NULL,
                      0, UINT16_MAX,
                      0,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  obj_properties[PROP_TABLE] =
    g_param_spec_int ("table",
                      NULL, NULL,
                      0, (TABLE_LAST-1),
                      TABLE_ON,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->set_property = rule_row_set_property;

  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);

  // Constructor
  G_OBJECT_CLASS (klass)->constructed = rule_row_constructed;

  // Signals
  obj_signals[SIGNAL_RULE_DELETED] =
    g_signal_new ("rule-deleted",
                  RULE_TYPE_ROW,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,            // no return value
                  1,                      // 1 argument
                  G_TYPE_UINT);           // table

  obj_signals[SIGNAL_ERROR] =
    g_signal_new ("error",
                  RULE_TYPE_ROW,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,            // no return value
                  1,                      // 1 argument
                  G_TYPE_STRING);         // error message
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
rule_row_new (Table    table,
              uint16_t rule_id)
{
  return RULE_ROW (g_object_new (RULE_TYPE_ROW,
                                 "table", table,
                                 "id", rule_id,
                                 NULL));
}
