/* error-dialog.h
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

#include <adwaita.h>

typedef enum
{
  ERROR_DIALOG_TYPE_DATABASE_ERROR,
  ERROR_DIALOG_TYPE_USER_GROUP_ERROR,

  ERROR_DIALOG_TYPE_LAST
} ErrorDialogType;

G_BEGIN_DECLS

#define ERROR_TYPE_DIALOG (error_dialog_get_type ())

G_DECLARE_FINAL_TYPE (ErrorDialog, error_dialog, ERROR, DIALOG, AdwWindow)

ErrorDialog *error_dialog_new (ErrorDialogType dialog_type);

G_END_DECLS
