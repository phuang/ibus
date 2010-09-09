/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
#ifndef __CONFIG_GCONF_H__
#define __CONFIG_GCONF_H__

#include <ibus.h>
#include <gconf/gconf-client.h>

#define IBUS_TYPE_CONFIG_GCONF	\
	(ibus_config_gconf_get_type ())

typedef struct _IBusConfigGConf IBusConfigGConf;
typedef struct _IBusConfigGConfClass IBusConfigGConfClass;

struct _IBusConfigGConf {
	IBusConfigService parent;
    GConfClient *client;
};

struct _IBusConfigGConfClass {
	IBusConfigServiceClass parent;

};


GType            ibus_config_gconf_get_type     (void);
IBusConfigGConf *ibus_config_gconf_new          (IBusConnection     *connection);

#endif
