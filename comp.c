#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compress(FILE *in, FILE *out, long max_size) {
    unsigned char *src = malloc(max_size);
    if (!src) return;
    
    long src_len = 0;
    int c;
    while (src_len < max_size && (c = fgetc(in)) != EOF) {
        src[src_len++] = (unsigned char)c;
    }

    // Explicitly mirror the 256-byte ring buffer index locations
    int history_pos[256];
    for (int i = 0; i < 256; i++) history_pos[i] = -1;

    long src_idx = 0;
    int buf_idx = 0; 

    while (src_idx < src_len) {
        unsigned char flags = 0xFF; 
        unsigned char chunk[32];
        int chunk_len = 0;

        for (int bit_idx = 0; bit_idx < 8 && src_idx < src_len; bit_idx++) {
            int best_len = 0;
            int best_absolute_offset = 0;

            for (int target_offset = 0; target_offset < 256; target_offset++) {
                int match_src_start = history_pos[target_offset];
                if (match_src_start == -1) continue;

                int len = 0;
                while (src_idx + len < src_len && 
                       src[match_src_start + len] == src[src_idx + len] && 
                       len < 18) {
                    len++;
                }

                if (len >= 3 && len > best_len) {
                    best_len = len;
                    best_absolute_offset = target_offset;
                }
            }

            if (best_len >= 3) {
                flags &= ~(1 << bit_idx);
                
                unsigned char b1 = (best_absolute_offset + 0xDE) & 0xFF;
                unsigned char b2 = (best_len - 3) & 0x0F;

                chunk[chunk_len++] = b1;
                chunk[chunk_len++] = b2;

                for (int i = 0; i < best_len; i++) {
                    history_pos[buf_idx] = (int)(src_idx + i);
                    buf_idx = (buf_idx + 1) & 0xFF;
                }
                src_idx += best_len;
            } else {
                flags |= (1 << bit_idx);
                chunk[chunk_len++] = src[src_idx];

                history_pos[buf_idx] = (int)src_idx;
                buf_idx = (buf_idx + 1) & 0xFF;
                src_idx++;
            }
        }
        fputc(flags, out);
        fwrite(chunk, 1, chunk_len, out);
    }

    free(src);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s [input file] [offset] [uncompressed size] [input rom]\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (!in) return 1;

    FILE *out = fopen(argv[4], "r+b");
    if (!out) out = fopen(argv[4], "wb");
    if (!out) { fclose(in); return 1; }

    long offset = strtol(argv[2], NULL, 0);
    long uncompressed_size = strtol(argv[3], NULL, 0);
    fseek(out, offset, SEEK_SET);

    compress(in, out, uncompressed_size);

    fclose(in);
    fclose(out);
    return 0;
}