/* rule-setup-dialog-add.c
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

#include "rule-setup-dialog-add.h"

struct _RuleSetupDialogAdd
{
  RuleSetupDialog     parent_instance;
};

G_DEFINE_FINAL_TYPE (RuleSetupDialogAdd, rule_setup_dialog_add, RULE_TYPE_SETUP_DIALOG)

static guint16
rule_setup_dialog_add_perform_action (Rule *rule)
{
  return rule_add (rule);
}

static void
rule_setup_dialog_add_class_init (RuleSetupDialogAddClass *klass)
{
  RuleSetupDialogClass *rule_setup_dialog_class = RULE_SETUP_DIALOG_CLASS (klass);

  rule_setup_dialog_class->perform_action = rule_setup_dialog_add_perform_action;
}

static void
rule_setup_dialog_add_init (RuleSetupDialogAdd *self)
{
  // Empty
}

RuleSetupDialogAdd *
rule_setup_dialog_add_new (Table table)
{
  return RULE_SETUP_DIALOG_ADD (g_object_new (RULE_TYPE_SETUP_DIALOG_ADD,
                                              "table", table,
                                              // Added rules are active by default
                                              "active", TRUE,
                                              "title", (table == TABLE_ON) ?
                                                       _("Add turn on rule") : _("Add turn off rule"),
                                              // translators: Rule Setup Dialog action button, for adding a rule
                                              "action-button-label", _("Add"),
                                              NULL));
}
