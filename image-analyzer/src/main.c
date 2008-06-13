/*
 *  MIRAGE Image Analyzer: Main
 *  Copyright (C) 2007-2008 Rok Mandeljc
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include "image-analyzer-application.h"


/******************************************************************************\
 *                                Main function                               *
\******************************************************************************/

gchar *file_to_open = NULL;

static GOptionEntry option_entries[] = {
    { "file", 'f', 0, G_OPTION_ARG_FILENAME, &file_to_open, "File to open", "path" },
    { NULL }
};

int main (int argc, char **argv) {
    GObject *application = NULL;
    GError *error = NULL;
    GOptionContext *option_context = NULL;
    gboolean succeeded = FALSE;

    /* Parse command line */
    option_context = g_option_context_new("- Mirage Image Analyzer");
    g_option_context_add_main_entries(option_context, option_entries, NULL);
    succeeded = g_option_context_parse(option_context, &argc, &argv, &error);
    g_option_context_free(option_context);
    
    if (!succeeded) {
        g_print("Failed to parse options: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    /* Initialize GType */
    g_type_init();
    
    /* Initialize Gtk */
    gtk_set_locale();
    gtk_init(&argc, &argv);
    
    /* Create application object */
    application = g_object_new(IMAGE_ANALYZER_TYPE_APPLICATION, NULL);
    
    /* Run application */
    if (!image_analyzer_application_run(IMAGE_ANALYZER_APPLICATION(application), &error)) {
        g_warning("Failed to run application: %s\n", error->message);
        g_error_free(error);
    }
        
    g_object_unref(application);
    
    return 0;
}