#!/bin/sed -f

/IBusObjectFlags/d;
/IBUS_OBJECT_FLAGS/d;
/IBUS_OBJECT_SET_FLAGS/d;
/IBUS_OBJECT_UNSET_FLAGS/d;
/IBus.*Private/d;

