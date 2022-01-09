
SRCDIR=src
OBJDIR=obj
SOURCES=$(wildcard $(SRCDIR)/*.c)
INCLUDE=include
INSIGHT_INCLUDE=$(SRCDIR)/INSIGHT/include
EXPORTED_FUNCTIONS=['_server_main', '_malloc', '_free']
INSIGHT=obj/insight.a

release: insight  out-directories
	emcc $(SOURCES) -DADEPT_INSIGHT_BUILD -I"$(INCLUDE)" -I"$(INSIGHT_INCLUDE)" -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s "EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS)" -s --pre-js js/pre.js $(INSIGHT) -lnodefs.js -o bin/insight_server.js

develop: insight out-directories
	emcc $(SOURCES) -DADEPT_INSIGHT_BUILD -I"$(INCLUDE)" -I"$(INSIGHT_INCLUDE)" -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s "EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS)" -s --pre-js js/pre.js --post-js js/post.js $(INSIGHT) -lnodefs.js -o bin/main.js -g

only-run:
	node bin/main.js

run: develop only-run

insight:
	$(MAKE) -C src/INSIGHT/

clean:
ifeq ($(OS), Windows_NT)
	del obj\*.* /Q
else
	rm -f 2> /dev/null obj/*.*
endif
	$(MAKE) -C src/INSIGHT/ clean

deepclean: clean

out-directories:
ifeq ($(OS), Windows_NT)
	@if not exist bin mkdir bin
	@if not exist bin mkdir obj
else
	@mkdir -p bin
	@mkdir -p obj
endif
