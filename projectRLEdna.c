#include <stdio.h>
#include <stdlib.h>

long getFileSize(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

int compressRLE(const char *infile, const char *outfile) {
    FILE *in = fopen(infile, "rb");
    FILE *out = fopen(outfile, "wb");

    if (!in || !out) return 0;  // failed

    int curr, prev = fgetc(in), count = 1;
    if (prev == EOF) return 0;

    while ((curr = fgetc(in)) != EOF) {
        if (curr == prev && count < 255) {
            count++;
        } else {
            fputc(count, out);
            fputc(prev, out);
            count = 1;
            prev = curr;
        }
    }
    fputc(count, out);
    fputc(prev, out);

    fclose(in);
    fclose(out);
    return 1;  // success
}

int decompressRLE(const char *infile, const char *outfile) {
    FILE *in = fopen(infile, "rb");
    FILE *out = fopen(outfile, "wb");
    if (!in || !out) return 0;

    int count, value;
    while ((count = fgetc(in)) != EOF && (value = fgetc(in)) != EOF) {
        for (int i = 0; i < count; i++)
            fputc(value, out);
    }

    fclose(in);
    fclose(out);
    return 1;  // success
}

int main() {
    int choice;
    char inputFile[100];

    while (1) {
        printf("\n====== RLE MENU ======\n");
        printf("1. Compress File\n");
        printf("2. Decompress File\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Enter input filename: ");
            scanf("%s", inputFile);

            if (compressRLE(inputFile, "compressed.rle")) {
                printf("\nCompression Successful!\n");
                printf("Original Size   : %ld bytes\n", getFileSize(inputFile));
                printf("Compressed Size : %ld bytes\n", getFileSize("compressed.rle"));
            } else {
                printf("\nCompression Failed! Check file name.\n");
            }
        }
        else if (choice == 2) {
            printf("Enter compressed filename: ");
            scanf("%s", inputFile);

            if (decompressRLE(inputFile, "decompressed.out")) {
                printf("\nDecompression Successful!\n");
                printf("Compressed Size   : %ld bytes\n", getFileSize(inputFile));
                printf("Decompressed Size : %ld bytes\n", getFileSize("decompressed.out"));
            } else {
                printf("\nDecompression Failed! Check file name.\n");
            }
        }
        else if (choice == 3) {
            printf("Exiting...\n");
            break;
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}