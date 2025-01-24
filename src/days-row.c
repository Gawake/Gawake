/* days-row.c
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

#include "days-row.h"

struct _DaysRow
{
  GtkBox                 parent_instance;

  // Widgets
  GtkToggleButton       *day_0;
  GtkToggleButton       *day_1;
  GtkToggleButton       *day_2;
  GtkToggleButton       *day_3;
  GtkToggleButton       *day_4;
  GtkToggleButton       *day_5;
  GtkToggleButton       *day_6;

  guint16                rule_id;
  Table                  table;
  gboolean               interactive;
};

// Properties
enum
{
  PROP_ID = 1,
  PROP_TABLE,
  PROP_INTERACTIVE,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

G_DEFINE_FINAL_TYPE (DaysRow, days_row, GTK_TYPE_BOX)

void
days_row_get_activated (DaysRow *self,
                        bool    *days)
{
  days[0] = gtk_toggle_button_get_active (self->day_0);
  days[1] = gtk_toggle_button_get_active (self->day_1);
  days[2] = gtk_toggle_button_get_active (self->day_2);
  days[3] = gtk_toggle_button_get_active (self->day_3);
  days[4] = gtk_toggle_button_get_active (self->day_4);
  days[5] = gtk_toggle_button_get_active (self->day_5);
  days[6] = gtk_toggle_button_get_active (self->day_6);
}

void
days_row_set_activated (DaysRow *self, bool days[7])
{
  gtk_toggle_button_set_active (self->day_0, days[0]);
  gtk_toggle_button_set_active (self->day_1, days[1]);
  gtk_toggle_button_set_active (self->day_2, days[2]);
  gtk_toggle_button_set_active (self->day_3, days[3]);
  gtk_toggle_button_set_active (self->day_4, days[4]);
  gtk_toggle_button_set_active (self->day_5, days[5]);
  gtk_toggle_button_set_active (self->day_6, days[6]);
}

static void
days_row_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  DaysRow *self = DAYS_ROW (object);

  switch (property_id)
    {
    case PROP_ID:
      self->rule_id = (uint16_t) g_value_get_int (value);
      break;

    case PROP_TABLE:
      self->table = (Table) g_value_get_int (value);
      break;

    case PROP_INTERACTIVE:
      self->interactive = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
days_row_restore_toggle_button_state (GtkButton *self,
                                      gpointer   user_data)
{
  GtkToggleButton *toggle_button = GTK_TOGGLE_BUTTON (self);
  gboolean active = gtk_toggle_button_get_active (toggle_button);

  // restore to previous state
  gtk_toggle_button_set_active (toggle_button, !active);
}

static void
days_row_constructed (GObject *gobject)
{
  DaysRow *self = DAYS_ROW (gobject);

  if (!self->interactive)
    {
      g_signal_connect (self->day_0,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_1,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_2,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_3,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_4,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_5,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);

      g_signal_connect (self->day_6,
                        "clicked",
                        G_CALLBACK (days_row_restore_toggle_button_state),
                        NULL);
    }
}

static void
days_row_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), DAYS_TYPE_ROW);

  G_OBJECT_CLASS (days_row_parent_class)->dispose (gobject);
}

static void
days_row_class_init (DaysRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/days-row.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_0);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_1);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_2);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_3);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_4);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_5);
  gtk_widget_class_bind_template_child (widget_class, DaysRow, day_6);

  // Properties
  obj_properties[PROP_ID] =
    g_param_spec_int ("id",
                      NULL, NULL,
                      0, UINT16_MAX,
                      0,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  obj_properties[PROP_TABLE] =
    g_param_spec_int ("table",
                      NULL, NULL,
                      0, (TABLE_LAST-1),
                      TABLE_ON,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  obj_properties[PROP_INTERACTIVE] =
    g_param_spec_boolean ("interactive",
                          NULL, NULL,
                          TRUE,
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_WRITABLE |
                          G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->set_property = days_row_set_property;
  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);

  G_OBJECT_CLASS (klass)->constructed = days_row_constructed;
  G_OBJECT_CLASS (klass)->dispose = days_row_dispose;
}

static void
days_row_init (DaysRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->rule_id = 0;
  self->table = TABLE_LAST;
}

DaysRow *
days_row_new (gboolean interactive)
{
  return DAYS_ROW (g_object_new (DAYS_TYPE_ROW,
                                 "interactive", interactive,
                                 NULL));
}
