cl /Fosimple.obj /c simple.c /nologo -EHsc -DNDEBUG /MD /I. /I../godot-headers
link /nologo /dll /out:..\project\gdnative\windows\libsimple.dll /implib:..\project\gdnative\windows\libsimple.lib simple.obj
