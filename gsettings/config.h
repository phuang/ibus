/* vim:set et sts=4: */
#ifndef __CONFIG_GCONF_H__
#define __CONFIG_GCONF_H__

#include <ibus.h>
#include <gio/gio.h>

#define IBUS_TYPE_CONFIG_GSETTINGS	\
	(ibus_config_gsettings_get_type ())

typedef struct _IBusConfigGSettings IBusConfigGSettings;
typedef struct _IBusConfigGSettingsClass IBusConfigGSettingsClass;

struct _IBusConfigGSettings {
	IBusConfigService parent;
    GSettings *settings;
};

struct _IBusConfigGSettingsClass {
	IBusConfigServiceClass parent;

};


GType                ibus_config_gconf_get_type     (void);
IBusConfigGSettings *ibus_config_gsettings_new      (IBusConnection     *connection);

#endif
