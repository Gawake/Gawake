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

#include <glib/gi18n.h>
#include <stdlib.h>

#include "error-dialog.h"

struct _ErrorDialog
{
  AdwWindow                  parent_instance;

  // Widgets
  AdwStatusPage             *status_page;
  GtkButton                 *action_button;
  GtkButton                 *ok_button;
  AdwToastOverlay           *toast;
  GtkLabel                  *extended_description;

  // Instance varables
  ErrorDialogType            dialog_type;
};

// Properties
enum
{
  PROP_DIALOG_TYPE = 1,

  N_PROPS
};

static GParamSpec *obj_properties[N_PROPS];

G_DEFINE_TYPE (ErrorDialog, error_dialog, ADW_TYPE_WINDOW)

static void
error_dialog_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  ErrorDialog *self = ERROR_DIALOG (object);

  switch (property_id)
    {
    case PROP_DIALOG_TYPE:
      self->dialog_type = (ErrorDialogType) g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
error_dialog_ok_button_clicked (GtkButton *button,
                                gpointer   user_data)
{
  ErrorDialog *self = ERROR_DIALOG (user_data);
  gtk_window_close (GTK_WINDOW (self));
}

static void
error_dialog_action_button_clicked (GtkButton *button,
                                    gpointer   user_data)
{
  ErrorDialog *self = ERROR_DIALOG (user_data);
  gint status = 1;

  gtk_widget_set_sensitive (GTK_WIDGET (self->action_button), FALSE);
  status = system ("pkexec /usr/sbin/usermod -aG gawake $USER");

  if (status == 0)
    adw_toast_overlay_add_toast (self->toast,
                                 adw_toast_new (_("User added successfully. Restart Gawake.")));
  else
    adw_toast_overlay_add_toast (self->toast,
                                 adw_toast_new (_("Failed to add user to group")));
}

static void
error_dialog_constructed (GObject *gobject)
{
  g_autofree gchar *extended_description_message = NULL;
  ErrorDialog *self = ERROR_DIALOG (gobject);

  switch (self->dialog_type)
    {
    case ERROR_DIALOG_TYPE_DATABASE_ERROR:
      adw_status_page_set_icon_name (self->status_page, "dialog-error-symbolic");
      adw_status_page_set_title (self->status_page, _("Database error"));
      adw_status_page_set_description (self->status_page, _("Failed to connect to the database"));
#if FLATPAK
      extended_description_message =
        g_strconcat (
          // Translatable
          _("Make sure that:\n"\
          /* TODO update url */
          "[1] Gawake server is installed. See <a href='https://github.com/Gawake'>this</a> for more information;\n"\
          "\n[2] You're in gawake group, by running:\n"),
          // Not translatable
          "<span font_family='monospace' font_weight='bold'>sudo usermod -aG gawake ",
          g_get_user_name (),
          "</span>",
          NULL
        );
      gtk_label_set_markup (self->extended_description, extended_description_message);
#endif
      break;

    case ERROR_DIALOG_TYPE_USER_GROUP_ERROR:
      gtk_button_set_label (self->action_button, _("Add user to group"));
      gtk_widget_set_visible (GTK_WIDGET (self->action_button), TRUE);
      g_signal_connect (self->action_button,
                        "clicked",
                        G_CALLBACK (error_dialog_action_button_clicked),
                        self);

      adw_status_page_set_icon_name (self->status_page, "system-users-symbolic");
      adw_status_page_set_title (self->status_page, _("System group error"));
      adw_status_page_set_description (self->status_page, _("You are not in the Gawake group"));
      break;

    case ERROR_DIALOG_TYPE_LAST:
    default:
      g_warning ("Invalid ErrorDialogType");
    }
}

static void
error_dialog_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), ERROR_TYPE_DIALOG);

  G_OBJECT_CLASS (error_dialog_parent_class)->dispose (gobject);
}

static void
error_dialog_class_init (ErrorDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/io/github/gawake/Gawake/error-dialog.ui");

  // Widgets
  gtk_widget_class_bind_template_child (widget_class, ErrorDialog, status_page);
  gtk_widget_class_bind_template_child (widget_class, ErrorDialog, action_button);
  gtk_widget_class_bind_template_child (widget_class, ErrorDialog, ok_button);
  gtk_widget_class_bind_template_child (widget_class, ErrorDialog, toast);
  gtk_widget_class_bind_template_child (widget_class, ErrorDialog, extended_description);

  // Properties
  obj_properties[PROP_DIALOG_TYPE] =
    g_param_spec_int ("type",
                      NULL, NULL,
                      0, (ERROR_DIALOG_TYPE_LAST-1),
                      ERROR_DIALOG_TYPE_DATABASE_ERROR,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_WRITABLE |
                      G_PARAM_STATIC_NAME);

  G_OBJECT_CLASS (klass)->set_property = error_dialog_set_property;
  g_object_class_install_properties (G_OBJECT_CLASS (klass),
                                     N_PROPS,
                                     obj_properties);

  G_OBJECT_CLASS (klass)->constructed = error_dialog_constructed;
  G_OBJECT_CLASS (klass)->dispose = error_dialog_dispose;
}

static void
error_dialog_init (ErrorDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  // Signals
  g_signal_connect (self->ok_button,
                    "clicked",
                    G_CALLBACK (error_dialog_ok_button_clicked),
                    self);
}

ErrorDialog *
error_dialog_new (ErrorDialogType dialog_type)
{
  return (g_object_new (ERROR_TYPE_DIALOG,
                        "type", dialog_type,
                        NULL));
}
