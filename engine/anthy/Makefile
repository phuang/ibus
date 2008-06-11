all: _anthy.so test

anthy.py anthy_wrap.c: anthy.i
	swig -python -I/usr/include $<
_anthy.so: anthy_wrap.c
	$(CC) -shared -o $@ $^ -fPIC `python-config --cflags --libs` `pkg-config anthy --cflags --libs`
test:
	PYTHONPATH=../.. python main.py
clean:
	$(RM) anthy.py anthy_wrap.c _anthy.so *.pyc
