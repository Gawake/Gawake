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
#include "rule-setup-dialog-edit.h"
#include "days-row.h"
#include "mode-row.h"
#include "time-chooser.h"

typedef struct
{
  AdwToastOverlay       *toast;
  GtkButton             *cancel_button;
  GtkButton             *action_button;
  AdwEntryRow           *name_entry;
  ModeRow               *mode_row;
  AdwBin                *days_row_bin;
  AdwBin                *conflicting_days_row_bin;
  GtkLabel              *conflicting_rule_title;
  GtkLabel              *conflicting_rule_time;
  GtkRevealer           *conflicting_rule_revealer;
  TimeChooser           *time_chooser;

  /* Instance variables */
  guint16               rule_id;
  Table                 table;
  gboolean              active;
  Mode                  mode;
  RuleTimeValidator    *rule_time_validator;
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
rule_setup_dialog_read_values (RuleSetupDialog *self,
                               Rule            *rule)
{
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);

  // Id
  rule->id = priv->rule_id;

  // Name
  g_snprintf (rule->name, RULE_NAME_LENGTH,
              "%s", gtk_editable_get_text (GTK_EDITABLE (priv->name_entry)));

  // Hour
  rule->hour = (uint8_t) time_chooser_get_hour24 (priv->time_chooser);

  // Minutes
  rule->minutes = (uint8_t) time_chooser_get_minutes (priv->time_chooser);

  // Days
  days_row_get_activated (DAYS_ROW (adw_bin_get_child (priv->days_row_bin)),
                          rule->days);

  // Active
  rule->active = priv->active;

  // Mode
  rule->mode = mode_row_get_mode (priv->mode_row);

  // Table
  rule->table = priv->table;
}

static void
rule_setup_dialog_set_conflicting_rule (RuleSetupDialog *self,
                                        guint16          conflicting_rule_id)
{
  Rule rule;
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  const gulong time_length = 6; // HH:MM AM'\n' == 9
  gchar rule_time[time_length];

  if (rule_get_single (conflicting_rule_id, priv->table, &rule) == EXIT_FAILURE)
    return;

  g_snprintf (rule_time, time_length,
              "%02d:%02d", rule.hour, rule.minutes);

  gtk_label_set_label (priv->conflicting_rule_title, rule.name);
  gtk_label_set_label (priv->conflicting_rule_time, rule_time);
  days_row_set_activated (DAYS_ROW (adw_bin_get_child (priv->conflicting_days_row_bin)),
                          rule.days);
  gtk_revealer_set_reveal_child (priv->conflicting_rule_revealer, TRUE);
}

static gboolean
rule_setup_dialog_check_for_conflicting_rule (RuleSetupDialog *self)
{
  Rule incoming_rule;
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  guint16 rule_id = 0;
  guint16 conflicting_rule_id = 0;

  rule_setup_dialog_read_values (self, &incoming_rule);

  // When the rule is being edited, only check for conflicting time if days or time was changed
  if (RULE_IS_SETUP_DIALOG_EDIT (self))
    rule_id = priv->rule_id;

  conflicting_rule_id = rule_validate_time (priv->rule_time_validator,
                                            rule_id,
                                            incoming_rule.hour,
                                            incoming_rule.minutes,
                                            incoming_rule.days);

  if (conflicting_rule_id == 0)
    {
      gtk_revealer_set_reveal_child (priv->conflicting_rule_revealer, FALSE);
      return FALSE;
    }
  else
    {
      rule_setup_dialog_set_conflicting_rule (self, conflicting_rule_id);
      return TRUE;
    }
}

/*
 * This is a generic funcion to detect that the user changed any value on the setup,
 * and then check if the change doesn't conflict any other rule. This callback is
 * connected to the widgets that affects the time signature.
 *
 * __widget_instance__ receives a pointer of the widget instance, do NOT use it
 */
static void
rule_setup_dialod_value_changed (gpointer __widget_instance__,
                                 gpointer user_data)
{
  rule_setup_dialog_check_for_conflicting_rule (RULE_SETUP_DIALOG (user_data));
}

static void
rule_setup_dialog_action_button_clicked (GtkButton *button,
                                         gpointer   user_data)
{
  Rule incoming_rule;
  RuleSetupDialog *self = RULE_SETUP_DIALOG (user_data);
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  RuleSetupDialogClass *klass = RULE_SETUP_DIALOG_GET_CLASS (self);

  rule_setup_dialog_read_values (self, &incoming_rule);

  // Validate rule values
  if (rule_validate_rule (&incoming_rule) == EXIT_FAILURE)
    {
      adw_toast_overlay_add_toast (priv->toast,
                                   adw_toast_new (_("Invalid rule attributes")));
      return;
    }

  // Check if the incoming rule is conflicting with another
  if (rule_setup_dialog_check_for_conflicting_rule (self))
    return;

  // Perform action if rule is valid
  incoming_rule.id = klass->perform_action (&incoming_rule);
  if (incoming_rule.id == 0)
    adw_toast_overlay_add_toast (priv->toast,
                                 adw_toast_new (_("Operation failed")));
  else
    rule_setup_dialog_emit_done (self,
                                 FALSE,
                                 priv->table,
                                 incoming_rule.id);
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

  // Init time validator
  priv->rule_time_validator = rule_validate_time_init (priv->table);
  if (priv->rule_time_validator == NULL)
    g_warning ("Failed to get RuleTimeValidator");

  // Reveal/hide mode
  gtk_widget_set_visible (GTK_WIDGET (priv->mode_row), (priv->table == TABLE_OFF));

  // If the rule has an id != 0, set the dialog fields (for editing it)
  if (priv->rule_id == 0)
    return;

  if (rule_get_single (priv->rule_id, priv->table, &rule) == EXIT_SUCCESS)
    {
      // Name
      gtk_editable_set_text (GTK_EDITABLE (priv->name_entry), rule.name);

      // Hour
      time_chooser_set_hour24 (priv->time_chooser, (gdouble) rule.hour);

      // Minutes
      time_chooser_set_minutes (priv->time_chooser, (gdouble) rule.minutes);

      // Active
      priv->active = rule.active;

      // Days
      days_row_set_activated (DAYS_ROW (adw_bin_get_child (priv->days_row_bin)),
                              rule.days);

      // Mode
      if (priv->table == TABLE_OFF)
        mode_row_set_mode (priv->mode_row, (guint) rule.mode);
    }
  else
    {
      adw_toast_overlay_add_toast (priv->toast,
                                   adw_toast_new (_("Failed to get rule attributes")));
    }
}

static void
rule_setup_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (rule_setup_dialog_parent_class)->finalize (object);
}

static void
rule_setup_dialog_dispose (GObject* object)
{
  RuleSetupDialog *self = RULE_SETUP_DIALOG (object);
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);

  rule_validate_time_finalize (&priv->rule_time_validator);

  G_OBJECT_CLASS (rule_setup_dialog_parent_class)->dispose (object);
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
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, name_entry);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, mode_row);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, conflicting_rule_revealer);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, days_row_bin);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, conflicting_days_row_bin);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, conflicting_rule_title);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, conflicting_rule_time);
  gtk_widget_class_bind_template_child_private (widget_class, RuleSetupDialog, time_chooser);

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

  G_OBJECT_CLASS (klass)->dispose = rule_setup_dialog_dispose;
  G_OBJECT_CLASS (klass)->finalize = rule_setup_dialog_finalize;
}

static void
rule_setup_dialog_init (RuleSetupDialog *self)
{
  RuleSetupDialogPrivate *priv = rule_setup_dialog_get_instance_private (self);
  DaysRow *days_row = NULL;
  DaysRow *conflicting_days_row = NULL;

  // Esure my custom widgets types
  g_type_ensure (TIME_TYPE_CHOOSER);
  g_type_ensure (MODE_TYPE_ROW);

  gtk_widget_init_template (GTK_WIDGET (self));

  // Widgets
  days_row = days_row_new (TRUE);
  conflicting_days_row = days_row_new (FALSE);

  adw_bin_set_child (priv->days_row_bin,
                     GTK_WIDGET (days_row));

  adw_bin_set_child (priv->conflicting_days_row_bin,
                     GTK_WIDGET (conflicting_days_row));

  // Signals
  g_signal_connect (priv->cancel_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_cancel_button_clicked),
                    self);

  g_signal_connect (priv->action_button,
                    "clicked",
                    G_CALLBACK (rule_setup_dialog_action_button_clicked),
                    self);

  g_signal_connect (priv->time_chooser,
                    "value-updated",
                    G_CALLBACK (rule_setup_dialod_value_changed),
                    self);

  g_signal_connect (days_row,
                    "value-updated",
                    G_CALLBACK (rule_setup_dialod_value_changed),
                    self);

  // Values
  priv->rule_id = 0;
  priv->table = TABLE_LAST;
  priv->active = TRUE;
  priv->mode = MODE_LAST;
  priv->rule_time_validator = NULL;

  mode_row_set_mode (priv->mode_row, (guint) MODE_OFF);
}

RuleSetupDialog *
rule_setup_dialog_new (void)
{
  return RULE_SETUP_DIALOG (g_object_new (RULE_TYPE_SETUP_DIALOG, NULL));
}
