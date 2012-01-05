# vim:set et sts=4 sw=4:
#!/usr/bin/env python

from xml.dom import minidom
import cgi

def simplfy_dom(node):
    name = node.nodeName
    children = {}
    if len(node.childNodes) == 1 and node.childNodes[0].nodeType == node.TEXT_NODE:
        return name, node.childNodes[0].nodeValue
    for child in node.childNodes:
        if child.nodeType != node.ELEMENT_NODE:
            continue
        child_name, child_value = simplfy_dom(child)
        if child_name not in children:
            children[child_name] = []
        children[child_name].append(child_value)
    return name, children

def parse_xml():
    filename = "/usr/share/X11/xkb/rules/evdev.xml"
    dom = minidom.parse(file(filename))
    name, root = simplfy_dom(dom)

    layouts = root['xkbConfigRegistry'][0]['layoutList'][0]['layout']
    for layout in layouts:
        config = layout['configItem'][0]
        name = config['name'][0]
        short_desc = config.get('shortDescription', [''])[0]
        desc = config.get('description', [''])[0]
        languages = config.get('languageList', [{}])[0].get('iso639Id', [])
        variants = layout.get('variantList', [{}])[0].get('variant', [])
        yield name, None, short_desc, desc, languages
        for variant in variants:
            variant_config = variant['configItem'][0]
            variant_name = variant_config['name'][0]
            variant_short_desc = variant_config.get('shortDescription', [''])[0]
            variant_desc = variant_config.get('description', [''])[0]
            variant_languages = variant_config.get('languageList', [{}])[0].get('iso639Id', [])
            if not isinstance(variant_languages, list):
                variant_languages = [variant_languages]
            yield name, variant_name, variant_short_desc, variant_desc, languages + variant_languages

def gen_xml():
    header = u"""<component>
	<name>org.freedesktop.IBus.Simple</name>
	<description>A table based simple engine</description>
	<exec>/home/penghuang/ibus/libexec/ibus-engine-simple</exec>
	<version>1.4.99.20120104</version>
	<author>Peng Huang &lt;shawn.p.huang@gmail.com&gt;</author>
	<license>GPL</license>
	<homepage>http://code.google.com/p/ibus</homepage>
	<textdomain>ibus</textdomain>
	<engines>"""
    engine = u"""\t\t<engine>
			<name>%s</name>
			<language>%s</language>
			<license>GPL</license>
			<author>Peng Huang &lt;shawn.p.huang@gmail.com&gt;</author>
			<layout>%s</layout>
			<longname>%s</longname>
			<description>%s</description>
			<rank>%d</rank>
		</engine>"""
    footer = u"""\t</engines>
</component>"""
    
    print header
    
    for name, vname, sdesc, desc, languages in parse_xml():
        if vname:
            ibus_name = "xkb:layout:%s-%s" % (name, vname)
            layout = "%s(%s)" % (name, vname)
        else:
            ibus_name = "xkb:layout:%s" % name
            layout = name
        for l in languages:
            desc = cgi.escape(desc)
            out = engine % (ibus_name + u"-" + l, l, layout, desc, desc, 99)
            print out.encode("utf8")
    
    print footer

if __name__ == "__main__":
    gen_xml()
