/* days-row.h
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

#include "database-connection/database-connection.h"

G_BEGIN_DECLS

#define DAYS_TYPE_ROW (days_row_get_type ())

G_DECLARE_FINAL_TYPE (DaysRow, days_row, DAYS, ROW, GtkBox)

DaysRow *days_row_new (Table    table,
                       guint16  rule_id,
                       gboolean interactive);
void days_row_get_activated (DaysRow *self, bool *days);

G_END_DECLS
