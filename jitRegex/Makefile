.PHONY: exe execute libjit_regex.so dub run clean
BUILD_TYPE = release
D_COMPILER = ldc2
DMD_PATH_ENV = DMD=/mnt/d/lenno/source/d2/install/linux/bin64
EXE_CMD = $(DMD_PATH_ENV) LD_LIBRARY_PATH=lib ./run

exe: run
	$(EXE_CMD)

execute:
	$(EXE_CMD)

libjit_regex.so: dub.json d/jitRegex.d
	$(DMD_PATH_ENV) dub build --compiler=$(D_COMPILER) --build=$(BUILD_TYPE) --force

dub: libjit_regex.so

run: libjit_regex.so cpp/run.cpp
	g++ -Wall -O2 cpp/run.cpp -o run -Llib -ljit_regex

clean:
	rm -rf lib/* cpp/*.o d/*.o run
