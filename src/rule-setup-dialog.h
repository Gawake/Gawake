/* rule-setup-dialog.h
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

#pragma once

#include <adwaita.h>

#define ALLOW_MANAGING_RULES
#include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

G_BEGIN_DECLS

#define RULE_TYPE_SETUP_DIALOG (rule_setup_dialog_get_type ())

G_DECLARE_FINAL_TYPE (RuleSetupDialog, rule_setup_dialog, RULE, SETUP_DIALOG, AdwWindow)

RuleSetupDialog *rule_setup_dialog_new_add (Table table);
RuleSetupDialog *rule_setup_dialog_new_edit (Table table, uint16_t rule_id);
void rule_setup_dialog_finish (RuleSetupDialog *self);

G_END_DECLS
