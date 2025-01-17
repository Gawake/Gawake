/* rule-setup-dialog-add.h
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

#include "rule-setup-dialog.h"

G_BEGIN_DECLS

#define RULE_TYPE_SETUP_DIALOG_ADD (rule_setup_dialog_add_get_type ())

G_DECLARE_FINAL_TYPE (RuleSetupDialogAdd, rule_setup_dialog_add, RULE, SETUP_DIALOG_ADD, RuleSetupDialog)

RuleSetupDialogAdd *rule_setup_dialog_add_new (Table table);

G_END_DECLS
