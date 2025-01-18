/* time-chooser.h
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

G_BEGIN_DECLS

#define TIME_TYPE_CHOOSER (time_chooser_get_type ())

G_DECLARE_FINAL_TYPE (TimeChooser, time_chooser, TIME, CHOOSER, GtkBox)

TimeChooser *time_chooser_new (void);

void time_chooser_set_hour (TimeChooser *self, gdouble hour);
void time_chooser_set_minutes (TimeChooser *self, gdouble minutes);
guint8 time_chooser_get_hour (TimeChooser *self);
guint8 time_chooser_get_minutes (TimeChooser *self);

G_END_DECLS
