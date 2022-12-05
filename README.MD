## nettop


### nim 版本

```

nim --mm:orc -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
69.6K

nim --mm:arc -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
65.6K

nim --mm:refc -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
73.7K

nim --mm:markAndSweep -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
69.7K

nim --mm:boehm -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
61.6K, alpine内运行失败：could not load: libgc.so.1

nim --mm:go -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
65.7K, alpine内运行失败：could not load: libgo.so

nim --mm:none -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
61.6K

nim --mm:regions -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
61.7K

```


### c 语言版本

gcc -static-libstdc++ -static-libgcc --static -Wl,-Bstatic,--gc-sections -Os -ffunction-sections -fdata-sections main.c -o nettop





