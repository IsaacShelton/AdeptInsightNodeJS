
SRCDIR=src
OBJDIR=obj
SOURCES=$(wildcard $(SRCDIR)/*.c)
INCLUDE=include
INSIGHT_INCLUDE=$(SRCDIR)/INSIGHT/include
EXPORTED_FUNCTIONS=['_server_main', '_malloc', '_free']
INSIGHT=obj/insight.a

develop: insight out-directories
	emcc $(SOURCES) -DADEPT_INSIGHT_BUILD -I"$(INCLUDE)" -I"$(INSIGHT_INCLUDE)" -s "EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS)" -s --pre-js js/pre.js --post-js js/post.js -lnodefs.js $(INSIGHT) -o bin/main.js

release:
	emcc $(SOURCES) -DADEPT_INSIGHT_BUILD -I"$(INCLUDE)" -I"$(INSIGHT_INCLUDE)" -s "EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS)" -s --pre-js js/pre.js -lnodefs.js $(INSIGHT) -o bin/AdeptInsightServer.js

only-run:
	node bin/main.js

run: release only-run

insight:
	$(MAKE) -C src/INSIGHT/emscripten

clean:
ifeq ($(OS), Windows_NT)
	del obj\*.* /Q
else
	rm -f 2> /dev/null obj/*.*
endif
	$(MAKE) -C src/INSIGHT/emscripten clean

deepclean: clean

out-directories:
ifeq ($(OS), Windows_NT)
	@if not exist bin mkdir bin
	@if not exist bin mkdir obj
else
	@mkdir -p bin
	@mkdir -p obj
endif
