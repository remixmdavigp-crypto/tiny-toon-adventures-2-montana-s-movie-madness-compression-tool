# tiny-toon-adventures-2-montana-s-movie-madness-compression-tool

a konami tiny toon adventures 2 montana's movie madness that not work with big sizes

# usage 

decompress

decomp [input rom] [offset] [uncompressed size] [output file]

compress and insert to rom 

comp [input file] [offset] [uncompressed size] [input rom]

example
decomp game.gb 0x10000 1728 busterbunny.bin

comp busterbunny.bin 0x10000 1728 game_edited.gb
