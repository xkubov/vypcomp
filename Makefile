LOGIN = login
PROJECT_FILES = CMakeLists.txt \
		src            \
		include        \
		tests          \
		Makefile       \
		division       \
		extensions

DOCUMENTATION = documentation.pdf

all:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release
	@cd build && $(MAKE) install
	@cp build/install/bin/vypcomp .

clean:
	@rm -rf build vypcomp

zip:
	zip -r $(LOGIN).zip $(PROJECT_FILES) $(DOCUMENTATION)
