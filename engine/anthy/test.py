#!/usr/bin/env python
# -*- coding: utf-8 -*-
import anthy
import sys

ctx = anthy.anthy_context ()
ctx._set_encoding (2)
if len(sys.argv) >= 2:
    ctx.set_string (sys.argv[1])
else:
    ctx.set_string ("かまぁく")
conv_stat = anthy.anthy_conv_stat ()
seg_stat = anthy.anthy_segment_stat ()
ctx.get_stat (conv_stat)
for i in range (0, conv_stat.nr_segment):
    ctx.get_segment_stat (i, seg_stat)
    buf = "          "
    i = ctx.get_segment (i, 0, buf, 10)
    print buf[:i]
# anthy.anthy_release_context (ctx)
ctx = None
