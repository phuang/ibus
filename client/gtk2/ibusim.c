/* vim:set et ts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <ibus.h>
#include "ibusimcontext.h"

#define IBUS_LOCALDIR ""
static const GtkIMContextInfo ibus_im_info = {
    "ibus",
    "iBus (Intelligent Input Bus)",
    "ibus",
    IBUS_LOCALDIR,
    "ja:ko:zh"
};

static const GtkIMContextInfo * info_list[] = {
    &ibus_im_info
};


void
im_module_init (GTypeModule *type_module)
{
    /* make module resident */
    g_module_make_resident (type_module);
    
    ibus_im_context_register_type(type_module);
}

void
im_module_exit (void)
{
}

GtkIMContext *
im_module_create (const gchar *context_id)
{
    if (g_strcmp0 (context_id, "ibus") == 0) {
        IBusIMContext *context;
        context = ibus_im_context_new ();
        return GTK_IM_CONTEXT(context);
    }
    return NULL;
}

void
im_module_list (const GtkIMContextInfo ***contexts,
                int                      *n_contexts)
{
    *contexts = info_list;
    *n_contexts = G_N_ELEMENTS (info_list);
}

