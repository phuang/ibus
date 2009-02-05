/* vim:set et sts=4: */
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __IBUS_SERIALIZABLE_H_
#define __IBUS_SERIALIZABLE_H_

#include "ibusobject.h"
#include "ibusmessage.h"

/*
 * Type macros.
 */

/* define IBusSerializable macros */
#define IBUS_TYPE_SERIALIZABLE             \
    (ibus_serializable_get_type ())
#define IBUS_SERIALIZABLE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_SERIALIZABLE, IBusSerializable))
#define IBUS_SERIALIZABLE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_SERIALIZABLE, IBusSerializableClass))
#define IBUS_IS_SERIALIZABLE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_SERIALIZABLE))
#define IBUS_IS_SERIALIZABLE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_SERIALIZABLE))
#define IBUS_SERIALIZABLE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_SERIALIZABLE, IBusSerializableClass))

#define ibus_serializable_set_attachment(o, k, v)  \
    ibus_serializable_set_qattachment (o, g_quark_from_string (k), v)
#define ibus_serializable_get_attachment(o, k, v)  \
    ibus_serializable_get_qattachment (o, g_quark_from_string (k))
#define ibus_serializable_remove_attachment(o, k)  \
    ibus_serializable_remove_qattachment (o, g_quark_from_string (k))

G_BEGIN_DECLS

typedef struct _IBusSerializable IBusSerializable;
typedef struct _IBusSerializableClass IBusSerializableClass;
/**
 * IBusSerializable:
 *
 * All the fields in the <structname>IBusSerializable</structname> structure are
 * prtivate to the #IBusSerializable and should never be accessed directly.
 */
struct _IBusSerializable {
  GObject parent;
  /* instance members */
  guint32 flags;
};

typedef gboolean    (* IBusSerializableSerializeFunc)   (IBusSerializable       *object,
                                                         IBusMessageIter        *iter);
typedef gboolean    (* IBusSerializableDeserializeFunc) (IBusSerializable       *object,
                                                         IBusMessageIter        *iter);
typedef gboolean    (* IBusSerializableCopyFunc)        (IBusSerializable       *dest,
                                                         const IBusSerializable *src);
struct _IBusSerializableClass {
    IBusObjectClass parent;

    /* signature */
    GString *signature;

    /* virtual table */
    gboolean    (* serialize)   (IBusSerializable       *object,
                                 IBusMessageIter        *iter);
    gboolean    (* deserialize) (IBusSerializable       *object,
                                 IBusMessageIter        *iter);
    gboolean    (* copy)        (IBusSerializable       *dest,
                                 const IBusSerializable *src);
    /*< private >*/
    /* padding */
    gpointer pdummy[5];
};

GType                ibus_serializable_get_type         (void);
IBusSerializable *   ibus_serializable_new              (void);
gboolean             ibus_serializable_set_qattachment  (IBusSerializable   *object,
                                                         GQuark              key,
                                                         const GValue       *value);
const GValue        *ibus_serializable_get_qattachment  (IBusSerializable   *object,
                                                         GQuark              key);
void                 ibus_serializable_remove_qattachment
                                                        (IBusSerializable   *object,
                                                         GQuark              key);
IBusSerializable    *ibus_serializable_copy             (IBusSerializable   *object);
gboolean             ibus_serializable_serialize        (IBusSerializable   *object,
                                                         IBusMessageIter    *iter);
IBusSerializable    *ibus_serializable_deserialize      (IBusMessageIter    *iter);

G_END_DECLS
#endif

