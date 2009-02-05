
/* Generated data (by glib-mkenums) */

#include "ibus.h"

/* enumerations from "ibusobject.h" */
GType
ibus_object_flags_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GFlagsValue values[] = {
            { IBUS_IN_DESTRUCTION, "IBUS_IN_DESTRUCTION", "in-destruction" },
            { IBUS_DESTROYED, "IBUS_DESTROYED", "destroyed" },
            { IBUS_RESERVED_1, "IBUS_RESERVED_1", "reserved-1" },
            { IBUS_RESERVED_2, "IBUS_RESERVED_2", "reserved-2" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static (g_intern_static_string ("IBusObjectFlags"), values);
    }
    return etype;
}

/* enumerations from "ibusattribute.h" */
GType
ibus_attr_type_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GEnumValue values[] = {
            { IBUS_ATTR_TYPE_UNDERLINE, "IBUS_ATTR_TYPE_UNDERLINE", "underline" },
            { IBUS_ATTR_TYPE_FOREGROUND, "IBUS_ATTR_TYPE_FOREGROUND", "foreground" },
            { IBUS_ATTR_TYPE_BACKGROUND, "IBUS_ATTR_TYPE_BACKGROUND", "background" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static (g_intern_static_string ("IBusAttrType"), values);
    }
    return etype;
}

GType
ibus_attr_underline_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GEnumValue values[] = {
            { IBUS_ATTR_UNDERLINE_NONE, "IBUS_ATTR_UNDERLINE_NONE", "none" },
            { IBUS_ATTR_UNDERLINE_SINGLE, "IBUS_ATTR_UNDERLINE_SINGLE", "single" },
            { IBUS_ATTR_UNDERLINE_DOUBLE, "IBUS_ATTR_UNDERLINE_DOUBLE", "double" },
            { IBUS_ATTR_UNDERLINE_LOW, "IBUS_ATTR_UNDERLINE_LOW", "low" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static (g_intern_static_string ("IBusAttrUnderline"), values);
    }
    return etype;
}

/* enumerations from "ibusproperty.h" */
GType
ibus_prop_type_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GEnumValue values[] = {
            { PROP_TYPE_NORMAL, "PROP_TYPE_NORMAL", "normal" },
            { PROP_TYPE_TOGGLE, "PROP_TYPE_TOGGLE", "toggle" },
            { PROP_TYPE_RADIO, "PROP_TYPE_RADIO", "radio" },
            { PROP_TYPE_MENU, "PROP_TYPE_MENU", "menu" },
            { PROP_TYPE_SEPARATOR, "PROP_TYPE_SEPARATOR", "separator" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static (g_intern_static_string ("IBusPropType"), values);
    }
    return etype;
}

GType
ibus_prop_state_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GEnumValue values[] = {
            { PROP_STATE_UNCHECKED, "PROP_STATE_UNCHECKED", "unchecked" },
            { PROP_STATE_CHECKED, "PROP_STATE_CHECKED", "checked" },
            { PROP_STATE_INCONSISTENT, "PROP_STATE_INCONSISTENT", "inconsistent" },
            { 0, NULL, NULL }
        };
        etype = g_enum_register_static (g_intern_static_string ("IBusPropState"), values);
    }
    return etype;
}

/* enumerations from "ibustypes.h" */
GType
ibus_modifier_type_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GFlagsValue values[] = {
            { IBUS_SHIFT_MASK, "IBUS_SHIFT_MASK", "shift-mask" },
            { IBUS_LOCK_MASK, "IBUS_LOCK_MASK", "lock-mask" },
            { IBUS_CONTROL_MASK, "IBUS_CONTROL_MASK", "control-mask" },
            { IBUS_MOD1_MASK, "IBUS_MOD1_MASK", "mod1-mask" },
            { IBUS_MOD2_MASK, "IBUS_MOD2_MASK", "mod2-mask" },
            { IBUS_MOD3_MASK, "IBUS_MOD3_MASK", "mod3-mask" },
            { IBUS_MOD4_MASK, "IBUS_MOD4_MASK", "mod4-mask" },
            { IBUS_MOD5_MASK, "IBUS_MOD5_MASK", "mod5-mask" },
            { IBUS_BUTTON1_MASK, "IBUS_BUTTON1_MASK", "button1-mask" },
            { IBUS_BUTTON2_MASK, "IBUS_BUTTON2_MASK", "button2-mask" },
            { IBUS_BUTTON3_MASK, "IBUS_BUTTON3_MASK", "button3-mask" },
            { IBUS_BUTTON4_MASK, "IBUS_BUTTON4_MASK", "button4-mask" },
            { IBUS_BUTTON5_MASK, "IBUS_BUTTON5_MASK", "button5-mask" },
            { IBUS_FORWARD_MASK, "IBUS_FORWARD_MASK", "forward-mask" },
            { IBUS_SUPER_MASK, "IBUS_SUPER_MASK", "super-mask" },
            { IBUS_HYPER_MASK, "IBUS_HYPER_MASK", "hyper-mask" },
            { IBUS_META_MASK, "IBUS_META_MASK", "meta-mask" },
            { IBUS_RELEASE_MASK, "IBUS_RELEASE_MASK", "release-mask" },
            { IBUS_MODIFIER_MASK, "IBUS_MODIFIER_MASK", "modifier-mask" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static (g_intern_static_string ("IBusModifierType"), values);
    }
    return etype;
}

GType
ibus_capabilite_get_type (void)
{
    static GType etype = 0;
    if (G_UNLIKELY(etype == 0)) {
        static const GFlagsValue values[] = {
            { IBUS_CAP_PREEDIT_TEXT, "IBUS_CAP_PREEDIT_TEXT", "preedit-text" },
            { IBUS_CAP_AUXILIARY_TEXT, "IBUS_CAP_AUXILIARY_TEXT", "auxiliary-text" },
            { IBUS_CAP_LOOKUP_TABLE, "IBUS_CAP_LOOKUP_TABLE", "lookup-table" },
            { IBUS_CAP_FOCUS, "IBUS_CAP_FOCUS", "focus" },
            { IBUS_CAP_PROPERTY, "IBUS_CAP_PROPERTY", "property" },
            { 0, NULL, NULL }
        };
        etype = g_flags_register_static (g_intern_static_string ("IBusCapabilite"), values);
    }
    return etype;
}


/* Generated data ends here */

