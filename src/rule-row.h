/* rule-row.h
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

#include <gtk/gtk.h>

#define ALLOW_MANAGING_RULES
#include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

G_BEGIN_DECLS

#define RULE_TYPE_ROW (rule_row_get_type ())

G_DECLARE_FINAL_TYPE (RuleRow, rule_row, RULE, ROW, GtkListBoxRow)

RuleRow *rule_row_new (Table table, uint16_t rule_id);
guint16 rule_row_get_id (RuleRow *self);
void rule_row_update_fields (RuleRow *self);

G_END_DECLS

