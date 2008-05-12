/* vim:set et ts=4: */
/* GIK - The G Input Toolkit
 * Copyright (C) 2008-2009 Huang Peng
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GIK_IM_CONTEXT_H_
#define __GIK_IM_CONTEXT_H_

#include <gtk/gtk.h>

/*
 * Type macros.
 */

G_BEGIN_DECLS
typedef struct _GikIMContext GikIMContext;
typedef struct _GikIMContextClass GikIMContextClass;
typedef struct _GikIMContextPrivate GikIMContextPrivate;

struct _GikIMContext {
  GtkIMContext parent;
  /* instance members */
  GikIMContextPrivate *priv;
};

struct _GikIMContextClass {
  GtkIMContextClass parent;
  /* class members */
};

GtkIMContext *gik_im_context_new (void);
void gik_im_context_register_type (GTypeModule *type_module);
void gik_im_context_shutdown (void);

G_END_DECLS
#endif

