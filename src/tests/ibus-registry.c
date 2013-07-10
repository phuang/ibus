#include <ibus.h>

int main()
{
#if !GLIB_CHECK_VERSION(2,35,0)
    g_type_init ();
#endif
    IBusRegistry *registry = ibus_registry_new ();
    g_object_unref (registry);
    return 0;
}
