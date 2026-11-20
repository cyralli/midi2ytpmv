all:
	xxd -i shaders/default.vert > shaders/default.vert.h
	xxd -i shaders/default.frag > shaders/default.frag.h
	gcc -DGL_GLEXT_PROTOTYPES src/main.c -o build -lEGL -lgbm -lGL -Iinclude -Ishaders -lm
