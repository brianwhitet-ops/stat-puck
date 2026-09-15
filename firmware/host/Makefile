CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2
CPPFLAGS = -I../include -I. -DSLAB_HOST=1
LDFLAGS = -lz

CORE = \
	../src/slab_fb.c \
	../src/slab_font.c \
	../src/slab_font_draw.c \
	../src/slab_ui.c \
	../src/slab_state.c \
	../src/slab_input.c \
	../src/slab_persist.c \
	../src/slab_ble.c \
	../src/slab_epd.c

.PHONY: all bench test visual goldens clean

all: bench test

bench: bench_render
test: test_state

bench_render: bench_main.c png_write.c $(CORE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ bench_main.c png_write.c $(CORE) $(LDFLAGS)

test_state: ../tests/test_state.c png_write.c $(CORE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ ../tests/test_state.c $(CORE) $(LDFLAGS)

visual: bench_render test_state
	./test_state
	./bench_render ../testdata/out ../testdata/extra
	python3 ../tests/compare_goldens.py ../testdata/out ../testdata/golden

goldens: bench_render
	./bench_render ../testdata/golden ../testdata/extra
	python3 ../tools/make_contact.py

clean:
	rm -f bench_render test_state
