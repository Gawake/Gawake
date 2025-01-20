/* rule-face.c
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

#include "rule-face.h"
#include "rule-row.h"
#include "rule-setup-dialog-edit.h"
#include "rule-setup-dialog-add.h"

struct _RuleFace
{
  AdwBin               parent_instance;

  /* Widgets */
  GtkStack            *stack;
  AdwStatusPage       *empty_view;
  GtkScrolledWindow   *list_view;
  GtkButton           *action_button;
  GtkListBox          *list_box;
  AdwToastOverlay     *toast;

  /* Instace variables */
  guint16              row_count;
  Table                table;
  RuleFaceType         type;
};

// Properties
enum
{
  PROP_TYPE = 1,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

G_DEFINE_TYPE (RuleFace, rule_face, ADW_TYPE_BIN)

static void
rule_face_set_empty_view (RuleFace *self)
{
  gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->empty_view));
}

static void
rule_face_set_list_view (RuleFace *self)
{
  gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->list_view));
}

static void
rule_face_check_for_empty_view (RuleFace *self)
{
  if (self->row_count == 0)
    rule_face_set_empty_view (self);
  else
    rule_face_set_list_view (self);
}

static void
rule_face_delete_rule (RuleRow *row,
                       guint     _table,
                       gpointer  user_data)
{
  RuleFace *self = RULE_FACE (user_data);

  gtk_list_box_remove (self->list_box, GTK_WIDGET (row));
  self->row_count--;
  rule_face_check_for_empty_view (self);

  g_clear_object (&row);
}

static void
rule_face_show_error_for_row (RuleRow     *row,
                              const gchar *error,
                              gpointer     user_data)
{
  RuleFace *self = RULE_FACE (user_data);

  adw_toast_overlay_add_toast (self->toast, adw_toast_new (error));
}

static void
rule_face_append_rule (RuleFace *self,
                       guint16   rule_id)
{
  RuleRow *row = rule_row_new (self->table, rule_id);

  g_signal_connect (row,
                    "rule-deleted",
                    G_CALLBACK (rule_face_delete_rule),
                    self);

  g_signal_connect (row,
                    "error",
                    G_CALLBACK (rule_face_show_error_for_row),
                    self);

  gtk_list_box_append (self->list_box, GTK_WIDGET (row));
  self->row_count++;
}

static void
rule_face_edit_rule (RuleSetupDialog *dialog,
                     gboolean         cancelled,
                     guint            _table,
                     guint            _rule_id,
                     gpointer         user_data)
{
  RuleRow *row = RULE_ROW (user_data);

  if (!cancelled)
    rule_row_update_fields (row);

  rule_setup_dialog_finish (dialog);
}

static void
rule_face_add_rule (RuleSetupDialog *dialog,
                    gboolean         cancelled,
                    guint            _table,
                    guint            _rule_id,
                    gpointer         user_data)
{
  RuleFace *self = RULE_FACE (user_data);
  guint16 rule_id = (guint16) _rule_id;

  if (!cancelled)
    {
      rule_face_append_rule (self, rule_id);
      rule_face_check_for_empty_view (self);
    }

  rule_setup_dialog_finish (dialog);
}

static void
rule_face_list_box_row_activated (GtkListBox    *list_box,
                                  GtkListBoxRow *row,
                                  gpointer       user_data)
{
  RuleFace *self = RULE_FACE (user_data);
  GtkWidget *parent = GTK_WIDGET (self);
  guint16 rule_id = rule_row_get_id (RULE_ROW (row));
  GtkWindow *dialog = NULL;

  dialog = GTK_WINDOW (rule_setup_dialog_edit_new (self->table, rule_id));

  g_signal_connect (dialog,
                    "done",
                    G_CALLBACK (rule_face_edit_rule),
                    RULE_ROW (row));

  while ((parent = gtk_widget_get_parent (parent)) != NULL)
    {
      g_debug ("Parent widget: %s", G_OBJECT_TYPE_NAME (parent));
      if (GTK_IS_WINDOW (parent))
        {
          gtk_window_set_transient_for (dialog, GTK_WINDOW (parent));
          break;
        }
    }

  gtk_window_present (dialog);
}

static void
rule_face_action_button_clicked (GtkButton *self,
                                 gpointer   user_data)
{
  RuleFace *face = RULE_FACE (user_data);
  rule_face_open_setup_add_dialog (face);
}

void
rule_face_open_setup_add_dialog (RuleFace *self)
{
  GtkWindow *dialog = NULL;
  GtkWidget *parent = GTK_WIDGET (self);

  dialog = GTK_WINDOW (rule_setup_dialog_add_new (self->table));

  g_signal_connect (dialog,
                    "done",
                    G_CALLBACK (rule_face_add_rule),
                    self);

  while ((parent = gtk_widget_get_parent (parent)) != NULL)
    {
      g_debug ("Parent widget: %s", G_OBJECT_TYPE_NAME (parent));
      if (GTK_IS_WINDOW (parent))
        {
          gtk_window_set_transient_for (dialog, GTK_WINDOW (parent));
          break;
        }
    }

  gtk_window_present (dialog);
}

static void
rule_face_populate_rules (RuleFace *self)
{
  Rule *rules = NULL;
  guint16 row_count = 0;

  if (rule_get_all (self->table, &rules, &row_count) == EXIT_FAILURE)
    {
      adw_toast_overlay_add_toast (self->toast, adw_toast_new (_("Failed to get rules")));
      return;
    }

  for (guint16 row_idx = 0; row_idx < row_count; row_idx++)
    rule_face_append_rule (self, rules[row_idx].id);

  free (rules);
}

static void
rule_face_constructed (GObject *gobject)
{
  RuleFace *self = RULE_FACE (gobject);
  G_OBJECT_CLASS (rule_face_parent_class)->constructed (gobject);

  // Set empty view icon
  switch (self->type)
    {
    case RULE_FACE_TYPE_TURN_ON:
      adw_status_page_set_icon_name (self->empty_view, "alarm-symbolic");
      break;

    case RULE_FACE_TYPE_TURN_OFF:
      adw_status_page_set_icon_name (self->empty_view, "system-shutdown-symbolic");
      break;

    case RULE_FACE_TYPE_LAST:
    default:
      return;
    }

  // Populate rules
  rule_face_populate_rules (self);

  rule_face_check_for_empty_view (self);
}

static void
rule_face_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  RuleFace *self = RULE_FACE (object);

  switch (property_id)
    {
    case PROP_TYPE:
      self->type = (RuleFaceType) g_value_get_int (value);
      self->table = (self->type == RULE_FACE_TYPE_TURN_ON) ? TABLE_ON : TABLE_OFF;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
rule_face_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), RULE_TYPE_FACE);

  G_OBJECT_CLASS (rule_face_parent_class)->dispose (gobject);
}

static void
rule_face_class_init (RuleFaceClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/rule-face.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, RuleFace, stack);
  gtk_widget_class_bind_template_child (widget_class, RuleFace, empty_view);
  gtk_widget_class_bind_template_child (widget_class, RuleFace, action_button);
  gtk_widget_class_bind_template_child (widget_class, RuleFace, list_box);
  gtk_widget_class_bind_template_child (widget_class, RuleFace, list_view);
  gtk_widget_class_bind_template_child (widget_class, RuleFace, toast);

  // Properties
  obj_properties[PROP_TYPE] =
    g_param_spec_int ("type",
                      NULL, NULL,
                      0,
                      (RULE_FACE_TYPE_LAST-1),
                      RULE_FACE_TYPE_TURN_ON,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->set_property = rule_face_set_property;
  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);

  // Constructor
  G_OBJECT_CLASS (klass)->constructed = rule_face_constructed;

  G_OBJECT_CLASS (klass)->dispose = rule_face_dispose;
}

static void
rule_face_init (RuleFace *self)
{
  self->row_count = 0;

  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->list_box,
                    "row-activated",
                    G_CALLBACK (rule_face_list_box_row_activated),
                    self);

  g_signal_connect (self->action_button,
                    "clicked",
                    G_CALLBACK (rule_face_action_button_clicked),
                    self);
}

RuleFace *
rule_face_new (RuleFaceType type)
{
  return RULE_FACE (g_object_new (RULE_TYPE_FACE,
                                  "type", type,
                                  NULL));
}
