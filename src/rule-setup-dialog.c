/* rule-setup-dialog.c
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
 * Signal to update a rule on ListBox
 */

#include <glib/gi18n.h>

#include "rule-setup-dialog.h"

typedef enum
{
  RULE_SETUP_DIALOG_TYPE_ADD,
  RULE_SETUP_DIALOG_TYPE_EDIT,
  RULE_SETUP_DIALOG_TYPE_LAST
} RuleSetupDialogType;

struct _RuleSetupDialog
{
  AdwWindow            parent_instance;

  /* Template widgets */
  AdwToastOverlay     *toast;
  GtkButton           *cancel_button;
  GtkButton           *action_button;
  AdwComboRow         *mode_row;
  AdwEntryRow         *name_entry;
  GtkSpinButton       *h_spinbutton;
  GtkSpinButton       *m_spinbutton;
  GtkToggleButton     *day_0;
  GtkToggleButton     *day_1;
  GtkToggleButton     *day_2;
  GtkToggleButton     *day_3;
  GtkToggleButton     *day_4;
  GtkToggleButton     *day_5;
  GtkToggleButton     *day_6;

  /* Instance variables */
  uint16_t             rule_id;
  Table                table;
  RuleSetupDialogType  dialog_type;
};

enum
{
  PROP_ID = 1,
  PROP_TABLE,
  PROP_DIALOG_TYPE,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

G_DEFINE_TYPE (RuleSetupDialog, rule_setup_dialog, ADW_TYPE_WINDOW)

static gboolean
rule_setup_dialog_show_leading_zeros (GtkSpinButton *spin,
                                      gpointer       data)
{
   gchar *text;
   gint value;

   value = gtk_spin_button_get_value_as_int (spin);
   text = g_strdup_printf ("%02d", value);
   gtk_editable_set_text (GTK_EDITABLE (spin), text);
   g_free (text);

   return TRUE;
}


static void
rule_setup_dialog_cancel_button_clicked (GtkButton *self,
                                         gpointer   user_data)
{
  gtk_window_close (GTK_WINDOW (user_data));
}

static void
rule_setup_dialog_action_button_clicked (GtkButton *self,
                                         gpointer   user_data)
{
  Rule rule;
  RuleSetupDialog *dialog = RULE_SETUP_DIALOG (user_data);
  adw_combo_row_get_selected_item (dialog->mode_row);

  // Id
  rule.id = dialog->rule_id;

  // Name
  g_snprintf (rule.name, RULE_NAME_LENGTH,
              "%s", gtk_editable_get_text (GTK_EDITABLE (dialog->name_entry)));

  // Hour
  rule.hour = (uint8_t) gtk_spin_button_get_value (dialog->h_spinbutton);

  // Minutes
  rule.minutes = (uint8_t) gtk_spin_button_get_value (dialog->m_spinbutton);

  // Days
  rule.days[0] = (bool) gtk_toggle_button_get_active (dialog->day_0);
  rule.days[1] = (bool) gtk_toggle_button_get_active (dialog->day_1);
  rule.days[2] = (bool) gtk_toggle_button_get_active (dialog->day_2);
  rule.days[3] = (bool) gtk_toggle_button_get_active (dialog->day_3);
  rule.days[4] = (bool) gtk_toggle_button_get_active (dialog->day_4);
  rule.days[5] = (bool) gtk_toggle_button_get_active (dialog->day_5);
  rule.days[6] = (bool) gtk_toggle_button_get_active (dialog->day_6);

  // Active
  if (dialog->dialog_type == RULE_SETUP_DIALOG_TYPE_ADD)
    rule.active = true;

  // Mode
  if (dialog->table == TABLE_OFF)
    rule.mode = (Mode) adw_combo_row_get_selected (dialog->mode_row);
  else
    rule.mode = MODE_LAST;

  // Table
  rule.table = dialog->table;

  // Validate and add/edit rule
  if (rule_validate_rule (&rule) == EXIT_SUCCESS)
    {
      if (dialog->dialog_type == RULE_SETUP_DIALOG_TYPE_ADD)
        rule_add (&rule);
      else
        rule_edit (&rule);

      gtk_window_close (GTK_WINDOW (dialog));
    }
  else
    {
      adw_toast_overlay_add_toast (dialog->toast,
                                   adw_toast_new (_("Invalid rule attributes")));
    }
}

static void
rule_setup_dialog_constructed (GObject *gobject)
{
  RuleSetupDialog *self = RULE_SETUP_DIALOG (gobject);
  G_OBJECT_CLASS (rule_setup_dialog_parent_class)->constructed (gobject);

  // Set title
  if (self->dialog_type == RULE_SETUP_DIALOG_TYPE_ADD)
    {
      // ...adding a rule
      gtk_window_set_title (GTK_WINDOW (self),
                            (self->table == TABLE_ON) ?
                            _("Add turn on rule") : _("Add turn off rule"));
    }
  else
    {
      // ...editing a rule
      gtk_window_set_title (GTK_WINDOW (self),
                            (self->table == TABLE_ON) ?
                            _("Edit turn on rule") : _("Edit turn off rule"));
    }

  // Action button label
  if (self->dialog_type == RULE_SETUP_DIALOG_TYPE_ADD)
    {
      // translators: 'add button' for the rule setup dialog
      gtk_button_set_label (self->action_button, _("Add"));
    }
  else
    {
      // translators: 'confirm button' for the rule setup dialog, when editing a rule
      gtk_button_set_label (self->action_button, _("Done"));
    }

  // Reveal/hide mode
  gtk_widget_set_visible (GTK_WIDGET (self->mode_row),
                          (self->table == TABLE_OFF));

  // If the dialog is to edit a rule, set all fields with the rule's values
  if (self->dialog_type == RULE_SETUP_DIALOG_TYPE_EDIT)
    {
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

      status = rule_get_single (self->rule_id, self->table, &rule);

      if (status == EXIT_SUCCESS)
        {
          // Name
          gtk_editable_set_text (GTK_EDITABLE (self->name_entry), rule.name);

          // Hour
          gtk_spin_button_set_value (self->h_spinbutton, (gdouble) rule.hour);

          // Minutes
          gtk_spin_button_set_value (self->m_spinbutton, (gdouble) rule.minutes);

          // Days
          gtk_toggle_button_set_active (self->day_0, (gboolean) rule.days[0]);
          gtk_toggle_button_set_active (self->day_1, (gboolean) rule.days[1]);
          gtk_toggle_button_set_active (self->day_2, (gboolean) rule.days[2]);
          gtk_toggle_button_set_active (self->day_3, (gboolean) rule.days[3]);
          gtk_toggle_button_set_active (self->day_4, (gboolean) rule.days[4]);
          gtk_toggle_button_set_active (self->day_5, (gboolean) rule.days[5]);
          gtk_toggle_button_set_active (self->day_6, (gboolean) rule.days[6]);

          // Mode
          if (self->table == TABLE_OFF)
            adw_combo_row_set_selected (self->mode_row, (guint) rule.mode);
        }
      else
        {
          adw_toast_overlay_add_toast (self->toast,
                                       adw_toast_new (_("Failed to get rule attributes")));
        }
    }
}

static void
rule_setup_dialog_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  RuleSetupDialog *self = RULE_SETUP_DIALOG (object);

  switch (property_id)
    {
    case PROP_ID:
      self->rule_id = (uint16_t) g_value_get_int (value);
      break;

    case PROP_TABLE:
      self->table = (Table) g_value_get_int (value);
      break;

    case PROP_DIALOG_TYPE:
      self->dialog_type = (RuleSetupDialogType) g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
rule_setup_dialog_class_init (RuleSetupDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/rule-setup-dialog.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, toast);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, cancel_button);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, action_button);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, mode_row);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, name_entry);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, h_spinbutton);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, m_spinbutton);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_0);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_1);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_2);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_3);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_4);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_5);
  gtk_widget_class_bind_template_child (widget_class, RuleSetupDialog, day_6);

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

  obj_properties[PROP_DIALOG_TYPE] =
    g_param_spec_int ("dialog-type",
                      NULL, NULL,
                      0, (RULE_SETUP_DIALOG_TYPE_LAST-1),
                      RULE_SETUP_DIALOG_TYPE_ADD,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->constructed = rule_setup_dialog_constructed;
  G_OBJECT_CLASS (klass)->set_property = rule_setup_dialog_set_property;

  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);
}

static void
rule_setup_dialog_init (RuleSetupDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->cancel_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_cancel_button_clicked),
                    self);

  g_signal_connect (self->action_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_action_button_clicked),
                    self);

  g_signal_connect (self->h_spinbutton,
                    "output",
                    G_CALLBACK (rule_setup_dialog_show_leading_zeros),
                    NULL);

  g_signal_connect (self->m_spinbutton,
                    "output",
                    G_CALLBACK (rule_setup_dialog_show_leading_zeros),
                    NULL);

  // Values
  self->rule_id = 0;
  self->table = TABLE_LAST; // (invalid)
  self->dialog_type = RULE_SETUP_DIALOG_TYPE_ADD;

  adw_combo_row_set_selected (self->mode_row, (guint) MODE_OFF);
}

RuleSetupDialog *
rule_setup_dialog_new_add (Table table)
{
  return RULE_SETUP_DIALOG (g_object_new (RULE_TYPE_SETUP_DIALOG,
                                          "table", table,
                                          "dialog-type", (gint) RULE_SETUP_DIALOG_TYPE_ADD,
                                          NULL));
}

RuleSetupDialog *
rule_setup_dialog_new_edit (Table     table,
                            uint16_t  rule_id)
{
  return RULE_SETUP_DIALOG (g_object_new (RULE_TYPE_SETUP_DIALOG,
                                          "table", table,
                                          "id", rule_id,
                                          "dialog-type", (gint) RULE_SETUP_DIALOG_TYPE_EDIT,
                                          NULL));
}
