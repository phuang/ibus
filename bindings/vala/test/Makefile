TARGETS = \
    ibus-engine-enchant \
    ibus-config \
    $(NULL)

all: $(TARGETS)

ibus_pkgname = ibus-1.0

ibus-engine-enchant: enchant.vala
	valac --vapidir .. --pkg $(ibus_pkgname) --pkg enchant $^ -C
	valac -g --vapidir .. --pkg $(ibus_pkgname) --pkg enchant $^ -o $@

ibus-config: config.vala
	valac --vapidir .. --pkg $(ibus_pkgname) --pkg gio-2.0 --pkg vala-0.10 $^ -C
	valac -g --vapidir .. --pkg $(ibus_pkgname) --pkg gio-2.0 --pkg vala-0.10 $^ -o $@

clean:
	$(RM) $(TARGETS)
