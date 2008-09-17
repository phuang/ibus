#include <glib-object.h>
#include <gconf/gconf-value.h>

extern GConfEntry* gconf_entry_copy (const GConfEntry *src);

GType pygconf_value_get_type (void);
GType pygconf_entry_get_type (void);
GType pygconf_schema_get_type (void);
GType pygconf_meta_info_get_type (void);

#define GCONF_TYPE_VALUE    (pygconf_value_get_type ())
#define GCONF_TYPE_ENTRY    (pygconf_entry_get_type ())
#define GCONF_TYPE_SCHEMA   (pygconf_schema_get_type ())
#define GCONF_TYPE_METAINFO (pygconf_meta_info_get_type ())
