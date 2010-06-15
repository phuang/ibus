/* vim:set et sts=4: */

#include <string.h>
#include <dbus/dbus.h>
#include <ibus.h>
#include "config.h"

#define GCONF_PREFIX "/desktop/ibus"

/* functions prototype */
static void	        ibus_config_gconf_class_init    (IBusConfigGConfClass   *klass);
static void	        ibus_config_gconf_init		    (IBusConfigGConf		*config);
static void	        ibus_config_gconf_destroy		(IBusConfigGConf		*config);
static gboolean     ibus_config_gconf_set_value     (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     const GValue           *value,
                                                     IBusError             **error);
static gboolean     ibus_config_gconf_get_value     (IBusConfigService      *config,
                                                     const gchar            *section,
                                                     const gchar            *name,
                                                     GValue                 *value,
                                                     IBusError             **error);
static gboolean     ibus_config_gconf_unset     (IBusConfigService      *config,
                                                 const gchar            *section,
                                                 const gchar            *name,
                                                 IBusError             **error);

static GConfValue   *_to_gconf_value                (const GValue           *value);
static void          _from_gconf_value              (GValue                 *value,
                                                     const GConfValue       *gvalue);

static IBusConfigServiceClass *parent_class = NULL;

GType
ibus_config_gconf_get_type (void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusConfigGConfClass),
		(GBaseInitFunc)		NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc)	ibus_config_gconf_class_init,
		NULL,
		NULL,
		sizeof (IBusConfigGConf),
		0,
		(GInstanceInitFunc)	ibus_config_gconf_init,
	};

	if (type == 0) {
		type = g_type_register_static (IBUS_TYPE_CONFIG_SERVICE,
									   "IBusConfigGConf",
									   &type_info,
									   (GTypeFlags) 0);
	}

	return type;
}

static void
ibus_config_gconf_class_init (IBusConfigGConfClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = (IBusConfigServiceClass *) g_type_class_peek_parent (klass);

	IBUS_OBJECT_CLASS (object_class)->destroy = (IBusObjectDestroyFunc) ibus_config_gconf_destroy;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->set_value = ibus_config_gconf_set_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->get_value = ibus_config_gconf_get_value;
    IBUS_CONFIG_SERVICE_CLASS (object_class)->unset = ibus_config_gconf_unset;
}

static void
_value_changed_cb (GConfClient     *client,
                   const gchar     *key,
                   GConfValue      *value,
                   IBusConfigGConf *config)
{
    gchar *p, *section, *name;
    GValue v =  { 0 };

    g_return_if_fail (key != NULL);
    g_return_if_fail (value != NULL);

    p = g_strdup (key);
    section = p + sizeof (GCONF_PREFIX);
    name = rindex (p, '/') + 1;
    *(name - 1) = '\0';


    _from_gconf_value (&v, value);
    ibus_config_service_value_changed ((IBusConfigService *) config,
                                       section,
                                       name,
                                       &v);
    g_free (p);
    g_value_unset (&v);
}

static void
ibus_config_gconf_init (IBusConfigGConf *config)
{
    config->client = gconf_client_get_default ();
    gconf_client_add_dir (config->client,
                          GCONF_PREFIX,
                          GCONF_CLIENT_PRELOAD_RECURSIVE,
                          NULL);
    g_signal_connect (config->client, "value-changed", G_CALLBACK (_value_changed_cb), config);
}

static void
ibus_config_gconf_destroy (IBusConfigGConf *config)
{
    if (config->client) {
        g_signal_handlers_disconnect_by_func (config->client, G_CALLBACK (_value_changed_cb), config);
        g_object_unref (config->client);
        config->client = NULL;
    }

	IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)config);
}

static GConfValue *
_to_gconf_value (const GValue *value)
{
    GConfValue *gv;
    GType type = G_VALUE_TYPE (value);

    switch (type) {
    case G_TYPE_STRING:
        {
            gv = gconf_value_new (GCONF_VALUE_STRING);
            gconf_value_set_string (gv, g_value_get_string (value));
        }
        break;
    case G_TYPE_INT:
        {
            gv = gconf_value_new (GCONF_VALUE_INT);
            gconf_value_set_int (gv, g_value_get_int (value));
        }
        break;
    case G_TYPE_UINT:
        {
            gv = gconf_value_new (GCONF_VALUE_INT);
            gconf_value_set_int (gv, g_value_get_uint (value));
        }
        break;
    case G_TYPE_BOOLEAN:
        {
            gv = gconf_value_new (GCONF_VALUE_BOOL);
            gconf_value_set_bool (gv, g_value_get_boolean (value));
        }
        break;
    case G_TYPE_DOUBLE:
        {
            gv = gconf_value_new (GCONF_VALUE_FLOAT);
            gconf_value_set_float (gv, g_value_get_double (value));
        }
        break;
    case G_TYPE_FLOAT:
        {
            gv = gconf_value_new (GCONF_VALUE_FLOAT);
            gconf_value_set_float (gv, g_value_get_float (value));
        }
        break;
    default:
        if (type == G_TYPE_VALUE_ARRAY) {

            GSList *l = NULL;
            GType list_type = G_TYPE_STRING;
            GValueArray *array = g_value_get_boxed (value);
            gint i;

            if (array && array->n_values > 0) {
                list_type = G_VALUE_TYPE (&(array->values[0]));
            }

            gv = gconf_value_new (GCONF_VALUE_LIST);

            switch (list_type) {
            case G_TYPE_STRING:
                gconf_value_set_list_type (gv, GCONF_VALUE_STRING); break;
            case G_TYPE_INT:
            case G_TYPE_UINT:
                gconf_value_set_list_type (gv, GCONF_VALUE_INT); break;
            case G_TYPE_BOOLEAN:
                gconf_value_set_list_type (gv, GCONF_VALUE_BOOL); break;
            case G_TYPE_FLOAT:
            case G_TYPE_DOUBLE:
                gconf_value_set_list_type (gv, GCONF_VALUE_FLOAT); break;
            default:
                g_assert_not_reached ();
            }

            for (i = 0; array && i < array->n_values; i++) {
                GConfValue *tmp;
                g_assert (G_VALUE_TYPE (&(array->values[i])) == list_type);
                tmp = _to_gconf_value (&(array->values[i]));
                l = g_slist_append (l, tmp);
            }
            gconf_value_set_list_nocopy (gv, l);
        }
        else
            g_assert_not_reached ();
    }
    return gv;
}

static void
_from_gconf_value (GValue           *value,
                   const GConfValue *gv)
{
    g_assert (value);
    g_assert (gv);

    switch (gv->type) {
    case GCONF_VALUE_STRING:
        g_value_init (value, G_TYPE_STRING);
        g_value_set_string (value, gconf_value_get_string (gv));
        return;
    case GCONF_VALUE_INT:
        g_value_init (value, G_TYPE_INT);
        g_value_set_int (value, gconf_value_get_int (gv));
        return;
    case GCONF_VALUE_FLOAT:
        g_value_init (value, G_TYPE_DOUBLE);
        g_value_set_double (value, gconf_value_get_float (gv));
        return;
    case GCONF_VALUE_BOOL:
        g_value_init (value, G_TYPE_BOOLEAN);
        g_value_set_boolean (value, gconf_value_get_bool (gv));
        return;
    case GCONF_VALUE_LIST:
        {
            g_value_init (value, G_TYPE_VALUE_ARRAY);

            GSList *list, *p;
            GValueArray *va;

            list = gconf_value_get_list (gv);
            va = g_value_array_new (g_slist_length (list));
            for (p = list; p != NULL; p = p->next) {
                GValue tmp = {0};
                _from_gconf_value (&tmp, (GConfValue *) p->data);
                g_value_array_append (va, &tmp);
            }

            g_value_take_boxed (value, va);
        }
        return;
    default:
        g_assert_not_reached ();
        break;
    }
}


static gboolean
ibus_config_gconf_set_value (IBusConfigService      *config,
                             const gchar            *section,
                             const gchar            *name,
                             const GValue           *value,
                             IBusError             **error)
{
    gchar *key;
    GConfValue *gv;
    GError *gerror = NULL;

    gv = _to_gconf_value (value);

    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, name);

    gconf_client_set (((IBusConfigGConf *)config)->client, key, gv, &gerror);
    g_free (key);
    gconf_value_free (gv);

    if (gerror != NULL) {
        if (error) {
            *error = ibus_error_new_from_text (DBUS_ERROR_FAILED, gerror->message);
            g_error_free (gerror);
        }
        return FALSE;
    }

    return TRUE;
}
static gboolean
ibus_config_gconf_get_value (IBusConfigService      *config,
                             const gchar            *section,
                             const gchar            *name,
                             GValue                 *value,
                             IBusError             **error)
{
    gchar *key;
    GConfValue *gv;

    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, name);

    gv = gconf_client_get (((IBusConfigGConf *) config)->client, key, NULL);
    g_free (key);

    if (gv == NULL) {
        *error = ibus_error_new_from_printf (DBUS_ERROR_FAILED,
                                             "Can not get value [%s->%s]", section, name);
        return FALSE;
    }

    _from_gconf_value (value, gv);
    gconf_value_free (gv);
    return TRUE;
}
static gboolean
ibus_config_gconf_unset (IBusConfigService      *config,
                         const gchar            *section,
                         const gchar            *name,
                         IBusError             **error)
{
    gchar *key;
    GError *gerror = NULL;

    key = g_strdup_printf (GCONF_PREFIX"/%s/%s", section, name);

    gconf_client_unset (((IBusConfigGConf *)config)->client, key, &gerror);
    g_free (key);

    if (gerror != NULL) {
        if (error) {
            *error = ibus_error_new_from_text (DBUS_ERROR_FAILED, gerror->message);
            g_error_free (gerror);
        }
        return FALSE;
    }

    return TRUE;
}

IBusConfigGConf *
ibus_config_gconf_new (IBusConnection *connection)
{
    IBusConfigGConf *config;

    config = (IBusConfigGConf *) g_object_new (IBUS_TYPE_CONFIG_GCONF,
                                               "path", IBUS_PATH_CONFIG,
                                               "connection", connection,
                                               NULL);
    return config;
}
