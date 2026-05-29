#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------
// Get File Size
// -----------------------------------------------------
long getFileSize(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
        return -1;
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return -1;
    }
    long size = ftell(file);
    fclose(file);
    return size;
}

// -----------------------------------------------------
// BMP RLE Compressor
// -----------------------------------------------------
int compressBMP(const char *inputFile, const char *outputFile)
{
    FILE *in = fopen(inputFile, "rb");
    if (!in)
    {
        printf("Error: Could not open input BMP '%s'.\n", inputFile);
        return 0;
    }

    FILE *out = fopen(outputFile, "wb");
    if (!out)
    {
        printf("Error: Could not open output file '%s'.\n", outputFile);
        fclose(in);
        return 0;
    }

    unsigned char header[54];
    size_t hread = fread(header, 1, 54, in);
    if (hread != 54)
    {
        printf("Error: Failed to read BMP header (got %zu bytes).\n", hread);
        fclose(in);
        fclose(out);
        return 0;
    }
    fwrite(header, 1, 54, out);

    long pixelDataSize = getFileSize(inputFile) - 54;
    if (pixelDataSize <= 0)
    {
        printf("Error: No pixel data found or wrong BMP file.\n");
        fclose(in);
        fclose(out);
        return 0;
    }

    unsigned char *data = (unsigned char *)malloc(pixelDataSize);
    if (!data)
    {
        printf("Error: Memory allocation failed.\n");
        fclose(in);
        fclose(out);
        return 0;
    }

    size_t read = fread(data, 1, pixelDataSize, in);
    if ((long)read != pixelDataSize)
    {
        printf("Warning: expected %ld bytes of pixel data but read %zu.\n", pixelDataSize, read);
        pixelDataSize = (long)read;
    }

    long i = 0;
    while (i < pixelDataSize)
    {
        unsigned char value = data[i];
        int count = 1;

        while (i + count < pixelDataSize && data[i + count] == value && count < 255)
        {
            count++;
        }

        if (fputc(value, out) == EOF || fputc((unsigned char)count, out) == EOF)
        {
            printf("Error: write failed during compression.\n");
            free(data);
            fclose(in);
            fclose(out);
            return 0;
        }

        i += count;
    }

    free(data);
    fclose(in);
    fclose(out);
    return 1;
}

// -----------------------------------------------------
// BMP RLE Decompressor
// -----------------------------------------------------
int decompressBMP(const char *inputFile, const char *outputFile)
{
    FILE *in = fopen(inputFile, "rb");
    if (!in)
    {
        printf("Error: Could not open compressed file '%s'.\n", inputFile);
        return 0;
    }

    FILE *out = fopen(outputFile, "wb");
    if (!out)
    {
        printf("Error: Could not open output BMP '%s'.\n", outputFile);
        fclose(in);
        return 0;
    }

    unsigned char header[54];
    size_t hread = fread(header, 1, 54, in);
    if (hread != 54)
    {
        printf("Error: Failed to read header from compressed file (got %zu bytes).\n", hread);
        fclose(in);
        fclose(out);
        return 0;
    }
    fwrite(header, 1, 54, out);

    // Read pairs (value, count) until EOF
    while (1)
    {
        int v = fgetc(in);
        if (v == EOF)
            break;
        int c = fgetc(in);
        if (c == EOF)
        {
            printf("Warning: truncated compressed data (odd number of bytes).\n");
            break;
        }

        for (int i = 0; i < c; i++)
        {
            if (fputc((unsigned char)v, out) == EOF)
            {
                printf("Error: write failed during decompression.\n");
                fclose(in);
                fclose(out);
                return 0;
            }
        }
    }

    fclose(in);
    fclose(out);
    return 1;
}

// -----------------------------------------------------
// Utility: safe fgets to strip newline
// -----------------------------------------------------
void inputString(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        buffer[0] = '\0';
        return;
    }
    // strip newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
}

// -----------------------------------------------------
// MAIN FUNCTION (menu-driven)
// -----------------------------------------------------
int main()
{
    char choice[8];
    char inputFile[260];
    char outputFile[260];

    printf("Simple BMP RLE Compressor/Decompressor\n");
    printf("======================================\n");

    while (1)
    {
        printf("\nMenu:\n");
        printf("1) Compress BMP\n");
        printf("2) Decompress BMP (from .rle)\n");
        printf("3) Exit\n");

        inputString("Choose an option (1-3): ", choice, sizeof(choice));

        if (strcmp(choice, "1") == 0)
        {
            inputString("Enter input BMP filename (e.g. input.bmp): ", inputFile, sizeof(inputFile));
            inputString("Enter output compressed filename (e.g. compressed.rle.bmp): ", outputFile, sizeof(outputFile));

            long before = getFileSize(inputFile);
            if (before == -1)
            {
                printf("Error: Input file not found: %s\n", inputFile);
                continue;
            }

            printf("Starting compression of '%s' ...\n", inputFile);
            if (compressBMP(inputFile, outputFile))
            {
                long after = getFileSize(outputFile);
                if (after == -1) after = 0;
                printf("Compression successful.\n");
                printf("Original Size   : %ld bytes\n", before);
                printf("Compressed Size : %ld bytes\n", after);
            }
            else
            {
                printf("Compression failed.\n");
            }
        }
        else if (strcmp(choice, "2") == 0)
        {
            inputString("Enter compressed filename (e.g. compressed.rle.bmp): ", inputFile, sizeof(inputFile));
            inputString("Enter output BMP filename to create (e.g. output.bmp): ", outputFile, sizeof(outputFile));

            printf("Starting decompression of '%s' ...\n", inputFile);
            if (decompressBMP(inputFile, outputFile))
            {
                long restored = getFileSize(outputFile);
                if (restored == -1) restored = 0;
                printf("Decompression successful.\n");
                printf("Restored File Size : %ld bytes\n", restored);
            }
            else
            {
                printf("Decompression failed.\n");
            }
        }
        else if (strcmp(choice, "3") == 0 || strcmp(choice, "exit") == 0)
        {
            printf("Exiting. Goodbye!\n");
            break;
        }
        else
        {
            printf("Invalid option. Please enter 1, 2 or 3.\n");
        }
    }

    return 0;
}
