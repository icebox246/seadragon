dragon: main.c
	gcc `pkg-config --cflags gtk4` -o dragon main.c `pkg-config --libs gtk4`

compile_flags.txt:
	pkg-config --cflags gtk4 | sed 's/ /\n/g' > compile_flags.txt
