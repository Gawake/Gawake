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

#include <glib/gi18n.h>

#include "rule-setup-dialog.h"

typedef struct
{
  AdwToastOverlay       *toast;
  GtkButton             *cancel_button;
  GtkButton             *action_button;
  GtkSpinButton         *h_spinbutton;
  GtkSpinButton         *m_spinbutton;
  AdwEntryRow           *name_entry;
  AdwComboRow           *mode_row;
  GtkToggleButton       *day_0;
  GtkToggleButton       *day_1;
  GtkToggleButton       *day_2;
  GtkToggleButton       *day_3;
  GtkToggleButton       *day_4;
  GtkToggleButton       *day_5;
  GtkToggleButton       *day_6;

  /* Instance variables */
  guint16               rule_id;
  Table                 table;
  gboolean              active;
  Mode                  mode;
} RuleSetupDialogPrivate;

// Properties
enum
{
  PROP_ID = 1,
  PROP_TABLE,
  PROP_ACTIVE,
  PROP_TITLE,
  PROP_ACTION_BUTTON_LABEL,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

// Signals
enum
{
  SIGNAL_DONE,

  N_SIGNALS
};

static guint obj_signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (RuleSetupDialog, rule_setup_dialog, ADW_TYPE_WINDOW)

// PUBLIC
void
rule_setup_dialog_finish (RuleSetupDialog *self)
{
  gtk_window_close (GTK_WINDOW (self));
}

static void
rule_setup_dialog_emit_done (RuleSetupDialog *self,
                             gboolean         cancelled,
                             Table            table,
                             guint16          rule_id)
{
  g_signal_emit (self,
                 obj_signals[SIGNAL_DONE],
                 0,
                 cancelled,
                 table,
                 rule_id);
}

static void
rule_setup_dialog_cancel_button_clicked (GtkButton *self,
                                         gpointer   user_data)
{
  rule_setup_dialog_emit_done (RULE_SETUP_DIALOG (user_data),
                               TRUE,
                               TABLE_LAST,
                               0);
}

static void
rule_setup_dialog_action_button_clicked (GtkButton *button,
                                         gpointer   user_data)
{
  Rule rule;
  RuleSetupDialog *self = RULE_SETUP_DIALOG (user_data);
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  RuleSetupDialogClass *klass = RULE_SETUP_DIALOG_GET_CLASS (self);

  // Id
  rule.id = priv->rule_id;

  // Name
  g_snprintf (rule.name, RULE_NAME_LENGTH,
              "%s", gtk_editable_get_text (GTK_EDITABLE (priv->name_entry)));

  // Hour
  rule.hour = (uint8_t) gtk_spin_button_get_value (priv->h_spinbutton);

  // Minutes
  rule.minutes = (uint8_t) gtk_spin_button_get_value (priv->m_spinbutton);

  // Days
  rule.days[0] = (bool) gtk_toggle_button_get_active (priv->day_0);
  rule.days[1] = (bool) gtk_toggle_button_get_active (priv->day_1);
  rule.days[2] = (bool) gtk_toggle_button_get_active (priv->day_2);
  rule.days[3] = (bool) gtk_toggle_button_get_active (priv->day_3);
  rule.days[4] = (bool) gtk_toggle_button_get_active (priv->day_4);
  rule.days[5] = (bool) gtk_toggle_button_get_active (priv->day_5);
  rule.days[6] = (bool) gtk_toggle_button_get_active (priv->day_6);

  // Active
  rule.active = priv->active;

  // Mode
  rule.mode = (Mode) adw_combo_row_get_selected (priv->mode_row);

  // Table
  rule.table = priv->table;

  // Validate and perform action
  if (rule_validate_rule (&rule) == EXIT_SUCCESS)
    {
      rule.id = klass->perform_action (&rule);
      if (rule.id == 0)
        adw_toast_overlay_add_toast (priv->toast,
                                     adw_toast_new (_("Operation failed")));
      else
        rule_setup_dialog_emit_done (self,
                                     FALSE,
                                     priv->table,
                                     rule.id);
    }
  else
    {
      adw_toast_overlay_add_toast (priv->toast,
                                   adw_toast_new (_("Invalid rule attributes")));
    }
}

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
rule_setup_dialog_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  RuleSetupDialog *self = RULE_SETUP_DIALOG (object);
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);

  switch (property_id)
    {
    case PROP_ID:
      priv->rule_id = (uint16_t) g_value_get_int (value);
      break;

    case PROP_TABLE:
      priv->table = (Table) g_value_get_int (value);
      break;

    case PROP_ACTIVE:
      priv->active = g_value_get_boolean (value);
      break;

    case PROP_TITLE:
      gtk_window_set_title (GTK_WINDOW (self), g_value_get_string (value));
      break;

    case PROP_ACTION_BUTTON_LABEL:
      gtk_button_set_label (priv->action_button, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static guint16
rule_setup_dialog_perform_action (Rule *rule)
{
  g_warning ("Action not implemented");
  return 0;
}

static void
rule_setup_dialog_constructed (GObject *gobject)
{
  RuleSetupDialog *self = RULE_SETUP_DIALOG (gobject);
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
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

  G_OBJECT_CLASS (rule_setup_dialog_parent_class)->constructed (gobject);

  // Reveal/hide mode
  gtk_widget_set_visible (GTK_WIDGET (priv->mode_row), (priv->table == TABLE_OFF));

  // If the rule has an id, set the dialog fields (for editing it)
  if (priv->rule_id == 0)
    return;

  if (rule_get_single (priv->rule_id, priv->table, &rule) == EXIT_SUCCESS)
    {
      // Name
      gtk_editable_set_text (GTK_EDITABLE (priv->name_entry), rule.name);

      // Hour
      gtk_spin_button_set_value (priv->h_spinbutton, (gdouble) rule.hour);

      // Minutes
      gtk_spin_button_set_value (priv->m_spinbutton, (gdouble) rule.minutes);

      // Active
      priv->active = rule.active;

      // Days
      gtk_toggle_button_set_active (priv->day_0, (gboolean) rule.days[0]);
      gtk_toggle_button_set_active (priv->day_1, (gboolean) rule.days[1]);
      gtk_toggle_button_set_active (priv->day_2, (gboolean) rule.days[2]);
      gtk_toggle_button_set_active (priv->day_3, (gboolean) rule.days[3]);
      gtk_toggle_button_set_active (priv->day_4, (gboolean) rule.days[4]);
      gtk_toggle_button_set_active (priv->day_5, (gboolean) rule.days[5]);
      gtk_toggle_button_set_active (priv->day_6, (gboolean) rule.days[6]);

      // Mode
      if (priv->table == TABLE_OFF)
        adw_combo_row_set_selected (priv->mode_row, (guint) rule.mode);
    }
  else
    {
      adw_toast_overlay_add_toast (priv->toast,
                                   adw_toast_new (_("Failed to get rule attributes")));
    }
}

static void
rule_setup_dialog_class_init (RuleSetupDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/rule-setup-dialog.ui");

  // Widgets
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, toast);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, cancel_button);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, action_button);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, h_spinbutton);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, m_spinbutton);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, name_entry);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, mode_row);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_0);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_1);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_2);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_3);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_4);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_5);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, day_6);

  // Properties
  /* Pass an id != 0 to set the rule values in the dialog (for editing the rule) */
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

  obj_properties[PROP_ACTIVE] =
    g_param_spec_boolean ("active",
                          NULL, NULL,
                          FALSE,
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_WRITABLE |
                          G_PARAM_STATIC_NAME);

  obj_properties[PROP_TITLE] =
    g_param_spec_string ("title",
                         NULL, NULL,
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_WRITABLE |
                         G_PARAM_STATIC_NAME);

  obj_properties[PROP_ACTION_BUTTON_LABEL] =
    g_param_spec_string ("action-button-label",
                         NULL, NULL,
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_WRITABLE |
                         G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->set_property = rule_setup_dialog_set_property;
  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);

  // Signals
  obj_signals[SIGNAL_DONE] =
    g_signal_new ("done",
                  RULE_TYPE_SETUP_DIALOG,
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,            // no return value
                  3,                      // 3 arguments
                  G_TYPE_BOOLEAN,         // cancelled
                  G_TYPE_UINT,            // table
                  G_TYPE_UINT);           // id

  // Methods
  klass->perform_action = rule_setup_dialog_perform_action;

  // Constructor
  G_OBJECT_CLASS (klass)->constructed = rule_setup_dialog_constructed;
}

static void
rule_setup_dialog_init (RuleSetupDialog *self)
{
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (priv->cancel_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_cancel_button_clicked),
                    self);

  g_signal_connect (priv->action_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_action_button_clicked),
                    self);

  g_signal_connect (priv->h_spinbutton,
                    "output",
                    G_CALLBACK (rule_setup_dialog_show_leading_zeros),
                    NULL);

  g_signal_connect (priv->m_spinbutton,
                    "output",
                    G_CALLBACK (rule_setup_dialog_show_leading_zeros),
                    NULL);

  // Values
  priv->rule_id = 0;
  priv->table = TABLE_LAST;
  priv->active = TRUE;
  priv->mode = MODE_LAST;

  adw_combo_row_set_selected (priv->mode_row, (guint) MODE_OFF);
}

RuleSetupDialog *
rule_setup_dialog_new (void)
{
  return RULE_SETUP_DIALOG (g_object_new (RULE_TYPE_SETUP_DIALOG, NULL));
}
