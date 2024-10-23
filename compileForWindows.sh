# This script requires the mingw-w64-gcc compilers.
# On Arch they can be installed with
#
# sudo pacman -S mingw-w64-gcc
#
i686-w64-mingw32-gcc G2ME.c glicko2.c entry_file.c fileops.c player_dir.c printing.c sorting.c -o G2ME32.exe
x86_64-w64-mingw32-gcc G2ME.c glicko2.c entry_file.c fileops.c player_dir.c printing.c sorting.c -o G2ME64.exe
