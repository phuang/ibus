/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
#ifndef __CONFIG_GCONF_H__
#define __CONFIG_GCONF_H__

#include <ibus.h>
#include <gconf/gconf-client.h>

#define IBUS_TYPE_CONFIG_GCONF	\
    (ibus_config_gconf_get_type ())
#define IBUS_CONFIG_GCONF(obj)            \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_CONFIG_GCONF, IBusConfigGConf))
#define IBUS_CONFIG_GCONF_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_CONFIG_GCONF, IBusConfigGConfClass))
#define IBUS_IS_CONFIG_GCONF(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_CONFIG_GCONF))
#define IBUS_IS_CONFIG_GCONF_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_CONFIG_GCONF))
#define IBUS_CONFIG_GCONF_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_CONFIG_GCONF, IBusConfigGConfClass))

typedef struct _IBusConfigGConf IBusConfigGConf;
typedef struct _IBusConfigGConfClass IBusConfigGConfClass;

GType            ibus_config_gconf_get_type     (void);
IBusConfigGConf *ibus_config_gconf_new          (GDBusConnection    *connection);

#endif
