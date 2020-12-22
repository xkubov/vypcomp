all:
	mkdir -p build
	cd build && cmake .. -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release
	cd build && $(MAKE) install
	cp build/install/bin/vypcomp .
