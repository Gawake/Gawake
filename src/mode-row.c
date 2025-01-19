/* mode-row.c
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

#include "mode-row.h"

struct _ModeRow
{
  AdwComboRow           parent_instance;
};

G_DEFINE_TYPE (ModeRow, mode_row, ADW_TYPE_COMBO_ROW)

Mode
mode_row_get_mode (ModeRow *self)
{
  return (Mode) adw_combo_row_get_selected (ADW_COMBO_ROW (self));
}

void
mode_row_set_mode (ModeRow *self, Mode mode)
{
  adw_combo_row_set_selected (ADW_COMBO_ROW (self), (guint) mode);
}

static void
mode_row_class_init (ModeRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/mode-row.ui");
}

static void
mode_row_init (ModeRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

ModeRow *
mode_row_new (void)
{
  return MODE_ROW (g_object_new (MODE_TYPE_ROW, NULL));
}
