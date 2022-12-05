nim:
	nim -d:release --opt:size c main


buildc:
	cd c && \
	gcc -static-libstdc++ -static-libgcc --static -Wl,-Bstatic,--gc-sections -Os -ffunction-sections -fdata-sections main.c -o nettop