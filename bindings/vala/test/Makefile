ibus-engine-vala: test.vala
	valac --vapidir .. --pkg ibus-2.0 --pkg enchant $^ -C
	valac -g --vapidir .. --pkg ibus-2.0 --pkg enchant $^ -o $@
