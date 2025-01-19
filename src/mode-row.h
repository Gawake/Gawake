/* mode-row.h
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

#include "database-connection/database-connection.h"

#include <adwaita.h>

G_BEGIN_DECLS

#define MODE_TYPE_ROW (mode_row_get_type ())

G_DECLARE_FINAL_TYPE (ModeRow, mode_row, MODE, ROW, AdwComboRow)

ModeRow *mode_row_new (void);
Mode mode_row_get_mode (ModeRow *self);
void mode_row_set_mode (ModeRow *self, Mode mode);

G_END_DECLS
