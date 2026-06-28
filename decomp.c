#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void decompress(FILE *in, FILE *out, long uncompressed_size) {
    unsigned char flags = 0;
    int bit_count = 0;
    
    // 256-byte history sliding ring window for Game Boy tile formats
    unsigned char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    int buf_idx = 0;
    long written = 0;

    while (written < uncompressed_size) {
        if (bit_count == 0) {
            int c = fgetc(in);
            if (c == EOF) break;
            flags = (unsigned char)c;
            bit_count = 8;
        }

        if (flags & 1) {
            int c = fgetc(in);
            if (c == EOF) break;
            
            buffer[buf_idx] = (unsigned char)c;
            fputc(c, out);
            buf_idx = (buf_idx + 1) & 0xFF;
            written++;
        } else {
            int b1 = fgetc(in);
            int b2 = fgetc(in);
            if (b1 == EOF || b2 == EOF) break;

            int length = (b2 & 0x0F) + 3;
            int offset = (b1 - 0xDE) & 0xFF;

            for (int i = 0; i < length && written < uncompressed_size; i++) {
                unsigned char byte = buffer[(offset + i) & 0xFF];
                
                buffer[buf_idx] = byte;
                fputc(byte, out);
                buf_idx = (buf_idx + 1) & 0xFF;
                written++;
            }
        }
        flags >>= 1;
        bit_count--;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s [input rom] [offset] [uncompressed size] [output file]\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) return 1;
    
    long offset = strtol(argv[2], NULL, 0);
    long uncompressed_size = strtol(argv[3], NULL, 0);
    fseek(in, offset, SEEK_SET);

    FILE *out = fopen(argv[4], "wb");
    if (!out) { fclose(in); return 1; }

    decompress(in, out, uncompressed_size);

    fclose(in);
    fclose(out);
    return 0;
}