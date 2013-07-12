#include <ibus.h>

int main()
{
    ibus_init ();

    IBusRegistry *registry = ibus_registry_new ();
    g_object_unref (registry);
    return 0;
}
