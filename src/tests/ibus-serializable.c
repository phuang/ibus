#include "ibus.h"

void g_variant_type_info_assert_no_infos (void);

void test_serializable (IBusSerializable *object)
{
    GVariant *variant;
    gchar *s1, *s2;

    variant = ibus_serializable_serialize (object);
    g_object_unref (object);
    g_variant_get_data (variant);
    s1 = g_variant_print (variant, TRUE);

    object = ibus_serializable_deserialize (variant);
    g_variant_unref (variant);

    variant = ibus_serializable_serialize (object);
    g_object_unref (object);
    g_variant_get_data (variant);
    s2 = g_variant_print (variant, TRUE);
    g_variant_unref (variant);

    g_assert_cmpstr (s1, ==, s2);
    g_free (s1);
    g_free (s2);
}

static void
test_varianttypeinfo (void)
{
    g_variant_type_info_assert_no_infos ();
}

static void
test_attr_list (void)
{
    IBusAttrList *list = ibus_attr_list_new ();
    ibus_attr_list_append (list, ibus_attribute_new (1, 1, 1, 2));
    ibus_attr_list_append (list, ibus_attribute_new (2, 1, 1, 2));
    ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));
    ibus_attr_list_append (list, ibus_attribute_new (3, 1, 1, 2));
    test_serializable ((IBusSerializable *)list);
    g_variant_type_info_assert_no_infos ();
}

static void
test_text (void)
{
    test_serializable ((IBusSerializable *)ibus_text_new_from_string ("Hello"));
    test_serializable ((IBusSerializable *)ibus_text_new_from_string ("Hello"));
    test_serializable ((IBusSerializable *)ibus_text_new_from_string ("Hello"));
    test_serializable ((IBusSerializable *)ibus_text_new_from_string ("Hello"));
    g_variant_type_info_assert_no_infos ();
}

static void
test_engine_desc (void)
{
    test_serializable ((IBusSerializable *)ibus_engine_desc_new ("Hello",
                                        "Hello Engine",
                                        "Hello Engine Desc",
                                        "zh",
                                        "GPLv2",
                                        "Peng Huang <shawn.p.huang@gmail.com>",
                                        "icon",
                                        "en"));
    g_variant_type_info_assert_no_infos ();
}

static void
test_lookup_table (void)
{
    IBusLookupTable *table;

    table = ibus_lookup_table_new (9, 0, TRUE, FALSE);
    test_serializable ((IBusSerializable *)table);

#if  1
    table = ibus_lookup_table_new (9, 0, TRUE, FALSE);
    ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Hello"));
    ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Cool"));
    test_serializable ((IBusSerializable *)table);
#endif

    table = ibus_lookup_table_new (9, 0, TRUE, FALSE);
    ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Hello"));
    ibus_lookup_table_append_candidate (table, ibus_text_new_from_static_string ("Cool"));
    ibus_lookup_table_append_label (table, ibus_text_new_from_static_string ("Hello"));
    ibus_lookup_table_append_label (table, ibus_text_new_from_static_string ("Cool"));
    test_serializable ((IBusSerializable *)table);
    g_variant_type_info_assert_no_infos ();
}

static void
test_property (void)
{
    IBusPropList *list = ibus_prop_list_new ();
    ibus_prop_list_append (list, ibus_property_new ("sub1",
                                                    PROP_TYPE_NORMAL,
                                                    ibus_text_new_from_static_string ("label_sub1"),
                                                    "icon_sub1",
                                                    ibus_text_new_from_static_string ("tooltip_sub1"),
                                                    TRUE,
                                                    TRUE,
                                                    PROP_STATE_UNCHECKED,
                                                    NULL));
    ibus_prop_list_append (list, ibus_property_new ("sub2",
                                                    PROP_TYPE_NORMAL,
                                                    ibus_text_new_from_static_string ("label_sub1"),
                                                    "icon_sub1",
                                                    ibus_text_new_from_static_string ("tooltip_sub1"),
                                                    TRUE,
                                                    TRUE,
                                                    PROP_STATE_UNCHECKED,
                                                    NULL));
    g_object_ref (list);
    test_serializable ((IBusSerializable *)list);
    test_serializable ((IBusSerializable *)ibus_property_new ("key",
                                                          PROP_TYPE_NORMAL,
                                                          ibus_text_new_from_static_string ("label"),
                                                          "icon",
                                                          ibus_text_new_from_static_string ("tooltip"),
                                                          TRUE,
                                                          TRUE,
                                                          PROP_STATE_UNCHECKED,
                                                          list));
    g_variant_type_info_assert_no_infos ();
}

static void
test_attachment (void)
{
    IBusText *text =  ibus_text_new_from_static_string ("main text");

    ibus_serializable_set_attachment ((IBusSerializable *)text,
                                      "key1",
                                      g_variant_new_int32 (100));

    ibus_serializable_set_attachment ((IBusSerializable *)text,
                                      "key2",
                                      g_variant_new_string ("value string"));

    ibus_serializable_set_attachment ((IBusSerializable *)text,
                                      "key3",
                                      g_variant_new ("(iuds)",1, 2, 3.333, "test value"));

    GVariant *variant = ibus_serializable_serialize ((IBusSerializable *)text);
    g_object_unref ((IBusSerializable *)text);

    IBusSerializable *object = ibus_serializable_deserialize (variant);
    g_variant_unref (variant);

    g_assert_cmpstr (((IBusText *)object)->text, ==, "main text");

    GVariant *newvalue1 = ibus_serializable_get_attachment (object, "key1");
    g_assert (newvalue1 != NULL);
    g_assert_cmpint (g_variant_get_int32 (newvalue1), ==, 100);

    GVariant *newvalue2 = ibus_serializable_get_attachment (object, "key2");
    g_assert (newvalue2 != NULL);
    g_assert_cmpstr (g_variant_get_string (newvalue2, NULL), ==, "value string");

    {
        GVariant *newvalue3 = ibus_serializable_get_attachment (object, "key3");
        g_assert (newvalue3 != NULL);
        gint32 i;
        guint32 u;
        gdouble d;
        const gchar *s;
        g_variant_get (newvalue3, "(iud&s)", &i, &u, &d, &s);
        g_assert_cmpint (i, ==, 1);
        g_assert_cmpuint (u, ==, 2);
        g_assert_cmpfloat (d, ==, 3.333);
        g_assert_cmpstr (s, ==, "test value");
    }

    g_object_unref (object);
    g_variant_type_info_assert_no_infos ();
}

gint
main (gint    argc,
      gchar **argv)
{
    g_mem_set_vtable (glib_mem_profiler_table);
	g_type_init ();
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/ibus/varianttypeinfo", test_varianttypeinfo);
    g_test_add_func ("/ibus/attrlist", test_attr_list);
    g_test_add_func ("/ibus/text", test_text);
    g_test_add_func ("/ibus/enginedesc", test_engine_desc);
    g_test_add_func ("/ibus/lookuptable", test_lookup_table);
    g_test_add_func ("/ibus/property", test_property);
    g_test_add_func ("/ibus/attachment", test_attachment);

    return g_test_run ();
}
