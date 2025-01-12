/* rule-face.h
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
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

G_BEGIN_DECLS

typedef enum
{
  RULE_FACE_TYPE_TURN_ON,
  RULE_FACE_TYPE_TURN_OFF,

  RULE_FACE_TYPE_LAST
} RuleFaceType;

#define RULE_TYPE_FACE (rule_face_get_type ())

G_DECLARE_FINAL_TYPE (RuleFace, rule_face, RULE, FACE, AdwBin)

RuleFace *rule_face_new (RuleFaceType type);
void rule_face_open_setup_add_dialog (RuleFace *self);

G_END_DECLS

