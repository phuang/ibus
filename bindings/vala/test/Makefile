TARGETS = \
    ibus-engine-enchant \
    ibus-config \
    $(NULL)

all: $(TARGETS)

ibus-engine-enchant: enchant.vala
	valac --vapidir .. --pkg IBus-1.0 --pkg enchant $^ -C
	valac -g --vapidir .. --pkg IBus-1.0 --pkg enchant $^ -o $@

ibus-config: config.vala
	valac --vapidir .. --pkg IBus-1.0 --pkg gio-2.0 --pkg vala-0.10 $^ -C
	valac -g --vapidir .. --pkg IBus-1.0 --pkg gio-2.0 --pkg vala-0.10 $^ -o $@

clean:
	$(RM) $(TARGETS)
