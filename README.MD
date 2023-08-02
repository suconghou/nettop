## nettop


### nim 版本

```

nim --mm:orc --threads:off -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main

nim --mm:arc --threads:off -d:release --passC:-flto --passL:-flto --passL:-s --passL:-static --opt:size c main
```


### c 语言版本

```
gcc -static-libstdc++ -static-libgcc --static -Wl,-Bstatic,--gc-sections -Os -ffunction-sections -fdata-sections main.c -o nettop
```

### c++ 版本


```
g++ -std=c++20 -static-libstdc++ -static-libgcc --static -Wl,-Bstatic,--gc-sections -Os -ffunction-sections -fdata-sections main.cpp -o nettop
```




