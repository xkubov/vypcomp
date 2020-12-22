LOGIN = login
PROJECT_FILES = CMakeLists.txt \
		src            \
		include        \
		deps           \
		tests          \
		Makefile       \
		division       \
		extensions

DOCUMENTATION = documentation.pdf

.PHONY: tests

all:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release
	@cd build && $(MAKE) install
	@cp build/install/bin/vypcomp .

tests:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=on
	@cd build && $(MAKE) install
	@build/install/bin/vypcomp-tests

clean:
	@rm -rf build vypcomp

zip:
	zip -r $(LOGIN).zip $(PROJECT_FILES) $(DOCUMENTATION)
