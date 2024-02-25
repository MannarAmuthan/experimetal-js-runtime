### Experimental Javascript Runtime


To Build (On Macos/Linux):

You need to install/build V8 and libuv in your machine, and provide these variables

```

## Example

export V8_INCLUDE_PATH=.../v8/v8/include
export V8_LIB_PATH=...v8/v8/out.gn/x64.release.sample/obj/
export LIBUV_INCLUDE_PATH=...libuv/include
export LIBUV_STATIC_LIB=...libuv/.libs/libuv.a
```

```
make main
```




