.PHONY: all
all: withdll.exe hook.dll mpris-server.dll

.PHONY: detours
detours:
	$(MAKE) -C detours-unix

.PHONY: withdll.exe
withdll.exe: detours
	$(MAKE) -C withdll
	cp withdll/bin/withdll.exe $@

.PHONY: hook.dll
hook.dll: detours
	$(MAKE) -C hook
	cp hook/bin/hook.dll $@

.PHONY: mpris-server.dll
mpris-server.dll:
	$(MAKE) -C mpris
	cp mpris/mpris-server.dll.so $< $@

.PHONY: clean
clean:
	make -C detours-unix clean
	make -C withdll clean
	make -C hook clean
	make -C mpris clean
	rm -f *.exe *.dll