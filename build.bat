if not exist "build/nul" (
  mkdir build
)

gcc -std=c99 -I inc -c src/entrypoint.c -o build/entrypoint.o
gcc -std=c99 -I inc -c src/file.c       -o build/file.o
gcc -std=c99 -I inc -c src/filemap.c    -o build/filemap.o
gcc -mconsole -o build/program.exe build/*.o