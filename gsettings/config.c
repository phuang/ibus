/* vim:set et sts=4: */

#include <string.h>
#include <dbus/dbus.h>
#include <ibus.h>
#include "config.h"

#define GCONF_PREFIX "/desktop/ibus"

/* functions prototype */
static void	        ibus_config_gsettings_class_init    (IBusConfigGSettingsClass   *klass);
static void	        ibus_config_gsettings_init		    (IBusConfigGSettings		*config);
static void	        ibus_config_gsettings_destroy		(IBusConfigGSettings		*config);
static gboolean     ibus_config_gsettings_set_value     (IBusConfigService      *config,
                                                         const gchar            *section,
                                                         const gchar            *name,
                                                         const GValue           *value,
                                                         IBusError             **error);
static gboolean     ibus_config_gsettings_get_value     (IBusConfigService      *config,
                                                         const gchar            *section,
                                                         const gchar            *name,
                                                         GValue                 *value,
                                                         IBusError             **error);
static gboolean     ibus_config_gsettings_unset         (IBusConfigService      *config,
                                                         const gchar            *section,
                                                         const gchar            *name,
                                                         IBusError             **error);

static GVariant     *_to_variant                        (const GValue           *value);
static void          _from_variant                      (GValue                 *value,
                                                         GVariant               *variant);

static IBusConfigServiceClass *parent_class = NULL;

GType
ibus_config_gsettings_get_type (void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusConfigGSettingsClass),
		(GBaseInitFunc)		NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc)	ibus_config_gsettings_class_init,
		NULL,
		NULL,
		sizeof (IBusConfigGSettings),
		0,
		(GInstanceInitFunc)	ibus_config_gsettings_init,
	};

	if (type == 0) {
		type = g_type_register_static (IBUS_TYPE_CONFIG_SERVICE,
									   "IBusConfigGSettings",
									   &type_info,
									   (GTypeFlags) 0);
	}

	return type;
}

static void
ibus_config_gsettings_class_init (IBusConfigGSettingsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = (IBusConfigServiceClass *) g_type_class_peek_parent (klass);

	IBUS_OBJECT_CLASS (object_class)->destroy = (IBusObjectDestroyFunc) ibus_config_gsettings_destroy;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->set_value = ibus_config_gsettings_set_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_value = ibus_config_gsettings_get_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->unset = ibus_config_gsettings_unset;
}

static void
_changed_cb (GSettings           *settings,
             const gchar         *key,
             IBusConfigGSettings *config)
{
    g_debug ("changed key = %s", key);
#if 0
    gchar *p, *section, *name;
    GValue v =  { 0 };

    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);

    p = g_strdup (key);
    section = p + sizeof (GCONF_PREFIX);
    name = rindex (p, '/') + 1;
    *(name - 1) = '\0';


    _from_gsettings_value (&v, value);
    ibus_config_service_value_changed ((IBusConfigService *) config,
                                       section,
                                       name,
                                       &v);
    g_free (p);
    g_value_unset (&v);
#endif
}

static void
ibus_config_gsettings_init (IBusConfigGSettings *config)
{
    config->settings = g_settings_new ("org.freedesktop.ibus");
    g_signal_connect (config->settings, "changed", G_CALLBACK (_changed_cb), config);
}

static void
ibus_config_gsettings_destroy (IBusConfigGSettings *config)
{
    if (config->settings) {
        g_object_unref (config->settings);
        config->settings = NULL;
    }

	IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)config);
}

static GVariant *
_to_variant (const GValue *value)
{
    GVariant *variant = NULL;
    GType type = G_VALUE_TYPE (value);

    switch (type) {
    case G_TYPE_STRING:
        {
            variant = g_variant_new_string (g_value_get_string (value));
        }
        break;
    case G_TYPE_INT:
        {
            variant = g_variant_new_int32 (g_value_get_int (value));
        }
        break;
    case G_TYPE_UINT:
        {
            variant = g_variant_new_uint32 (g_value_get_uint (value));
        }
        break;
    case G_TYPE_BOOLEAN:
        {
            variant = g_variant_new_boolean (g_value_get_boolean (value));
        }
        break;
    case G_TYPE_DOUBLE:
        {
            variant = g_variant_new_double (g_value_get_double (value));
        }
        break;
    case G_TYPE_FLOAT:
        {
            variant = g_variant_new_double (g_value_get_float (value));
        }
        break;
    default:
        if (type == G_TYPE_VALUE_ARRAY) {
            gint i;
            GType list_type = G_TYPE_STRING;
            GValueArray *varray = (GValueArray *)g_value_get_boxed (value);
            GArray *array = g_array_new (TRUE, TRUE, sizeof (GVariant *));

            for (i = 0; varray && i < varray->n_values; i++) {
                GVariant *subvariant;
                g_assert (G_VALUE_TYPE (&(varray->values[i])) == list_type);
                subvariant = _to_variant (&(varray->values[i]));
                g_array_append_val (array, subvariant);
            }
            
            switch (list_type) {
            case G_TYPE_STRING:
                variant = g_variant_new_array (G_VARIANT_TYPE_STRING, (GVariant **)array->data, array->len); break;
            case G_TYPE_INT:
                variant = g_variant_new_array (G_VARIANT_TYPE_INT32, (GVariant **)array->data, array->len); break;
            case G_TYPE_UINT:
                variant = g_variant_new_array (G_VARIANT_TYPE_UINT32, (GVariant **)array->data, array->len); break;
            case G_TYPE_BOOLEAN:
                variant = g_variant_new_array (G_VARIANT_TYPE_BOOLEAN, (GVariant **)array->data, array->len); break;
            case G_TYPE_FLOAT:
            case G_TYPE_DOUBLE:
                variant = g_variant_new_array (G_VARIANT_TYPE_DOUBLE, (GVariant **)array->data, array->len); break;
            default:
                g_assert_not_reached ();
            }
            GVariant **pv = (GVariant **)g_array_free (array, FALSE);
            GVariant **p = (GVariant **)g_array_free (array, FALSE);
            while (*p != NULL) {
                g_variant_unref (*p);
                p ++;
            }
            g_free (pv);
        }
        else
            g_assert_not_reached ();
        break;
    }
    return variant;
}

static void
_from_variant (GValue   *value,
               GVariant *variant)
{
    g_assert (value);
    g_assert (variant);

    if (g_variant_get_type (variant) == G_VARIANT_TYPE_STRING) {
        g_value_init (value, G_TYPE_STRING);
        g_value_set_string (value, g_variant_get_string (variant, NULL));
        return;
    }
    if (g_variant_get_type (variant) == G_VARIANT_TYPE_INT32) {
        g_value_init (value, G_TYPE_INT);
        g_value_set_int (value, g_variant_get_int32 (variant));
        return;
    }
    if (g_variant_get_type (variant) == G_VARIANT_TYPE_UINT32) {
        g_value_init (value, G_TYPE_UINT);
        g_value_set_uint (value, g_variant_get_uint32 (variant));
        return;
    }
    if (g_variant_get_type (variant) == G_VARIANT_TYPE_DOUBLE) {
        g_value_init (value, G_TYPE_DOUBLE);
        g_value_set_double (value, g_variant_get_double (variant));
        return;
    }
    if (g_variant_get_type (variant) == G_VARIANT_TYPE_BOOLEAN) {
        g_value_init (value, G_TYPE_BOOLEAN);
        g_value_set_boolean (value, g_variant_get_boolean (variant));
        return;
    }
    if (g_variant_get_type (variant) == G_VARIANT_TYPE_ARRAY) {
            g_value_init (value, G_TYPE_VALUE_ARRAY);

            GValueArray *va;
            GValue sv = {0};
            gint i;
            va = g_value_array_new (g_variant_n_children (variant));
            for (i = 0; i < g_variant_n_children (variant); i++) {
                GVariant *subvariant = g_variant_get_child_value (variant, i);
                _from_variant (&sv, subvariant);
                g_value_array_append (va, &sv);
            }
            g_value_take_boxed (value, va);
        return;
    }
    g_assert_not_reached ();
}

gchar *
_refine_name (const gchar *name)
{
    gchar *newname, *p;
    p = newname = g_strdup (name);

    while (*p != 0) {
        if (*p == '_') *p = '-';
        p++;
    }
    return newname;
}

static gboolean
ibus_config_gsettings_set_value (IBusConfigService      *config,
                                 const gchar            *section,
                                 const gchar            *name,
                                 const GValue           *value,
                                 IBusError             **error)
{
    gchar *key;
    GVariant *variant;
    gboolean retval;

    g_debug ("set %s:%s", section, name);
    variant = _to_variant (value);
    if (variant == NULL)
        return TRUE;
    
    gchar *p = _refine_name (name);
    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, p);
    g_free (p);

    retval = g_settings_set_value (((IBusConfigGSettings *)config)->settings, key, variant);
    g_free (key);
    g_debug ("%d", retval);

    g_variant_unref (variant);

    return TRUE;
}
static gboolean
ibus_config_gsettings_get_value (IBusConfigService      *config,
                                 const gchar            *section,
                                 const gchar            *name,
                                 GValue                 *value,
                                 IBusError             **error)
{
    gchar *key;
    GVariant *variant;

    gchar *p = _refine_name (name);
    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, name);

    variant = g_settings_get_value (((IBusConfigGSettings *) config)->settings, p);
    g_free (p);
    g_free (key);

    if (variant == NULL) {
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not get value [%s->%s]", section, name);
        return FALSE;
    }

    _from_variant (value, variant);
    g_variant_unref (variant);
    
    return TRUE;
}

static gboolean
ibus_config_gsettings_unset (IBusConfigService      *config,
                             const gchar            *section,
                             const gchar            *name,
                             IBusError             **error)
{
#if 0
    gchar *key;
    GError *gerror = NULL;

    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, name);

    gsettings_client_unset (((IBusConfigGSettings *)config)->client, key, &gerror);
    g_free (key);

    if (gerror != NULL) {
        if (error) {
            *error = ibus_error_new_from_text (DBUS_ERROR_FAILED, gerror->message);
            g_error_free (gerror);
        }
        return FALSE;
    }

#endif
    return TRUE;
}

IBusConfigGSettings *
ibus_config_gsettings_new (IBusConnection *connection)
{
    IBusConfigGSettings *config;

    config = (IBusConfigGSettings *) g_object_new (IBUS_TYPE_CONFIG_GSETTINGS,
                                                   "path", IBUS_PATH_CONFIG,
                                                   "connection", connection,
                                                   NULL);
    return config;
}
