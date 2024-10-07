#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

size_t read_file(const char *filename, char **buffer)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "[Brainf*ck] Can't open file %s: %s\n", filename, strerror(errno));
        return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    if (size == 0)
    {
        fprintf(stderr, "[Brainf*ck] Nothing to process %s\n", filename);
        return 0;
    }

    *buffer = (char*)malloc(size);

    if (fread(*buffer, 1, size, file) != size)
    {
        fprintf(stderr, "[Brainf*ck] Can't read file %s\n", filename);
        fclose(file);
        return 0;
    }

    fclose(file);
    return size;
}

void write(FILE *file, const char *data)
{
    fwrite(data, strlen(data), 1, file);
}

void write_header(FILE* file)
{
    write(file, "#include<stdio.h>\n#include<string.h>\nint main(){");
    write(file, "int stk[30000];memset(stk,0,sizeof(stk));int sp=0;");
}

void write_body(FILE* file, char* input, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        switch (input[i])
        {
            case '>': write(file, "sp++;"); break;
            case '<': write(file, "sp--;"); break;
            case '+': write(file, "stk[sp]++;"); break;
            case '-': write(file, "stk[sp]--;"); break;
            case '.': write(file, "putchar(stk[sp]);"); break;
            case ',': write(file, "stk[sp]=getchar();"); break;
            case '[': write(file, "while(stk[sp]!=0){"); break;
            case ']': write(file, "}"); break;
        }
    }

    write(file, "return 0;}");
}

void simulate(char* input, size_t length)
{
    int stack[30000];
    memset(stack, 0, sizeof(stack));

    int cur = 0;
    int b = 0;

    while (*input)
    {
        switch (*input)
        {
            case '>': cur++; break;
            case '<': cur--; break;
            case '+': stack[cur]++; break;
            case '-': stack[cur]--; break;
            case '.': putchar(stack[cur]); break;
            case ',': stack[cur] = getchar(); break;
            
            case '[':
            {
                if (!stack[cur])
                {
                    b++;
                    while (b)
                    {
                        switch (*(++input))
                        {
                            case '[': b++; break;
                            case ']': b--; break;
                        }
                    }
                }
            }
            break;

            case ']':
            {
                if (stack[cur])
                {
                    b++;
                    while (b)
                    {
                        switch (*(--input))
                        {
                            case '[': b--; break;
                            case ']': b++; break;
                        }
                    }

                    input--;
                }
            }
            break;

        }

        input++;
    }
}

void compile(char* input, size_t length)
{
    FILE *output = fopen("./build/output.c", "wb");

    if (output == NULL)
    {
        fprintf(stderr, "[Brainf*ck] Can't open ./build/output.c file!\n");
        return;
    }
    
    printf("[Brainf*ck] Generating C file...\n");

    write_header(output);
    write_body(output, input, length);

    fclose(output);

    printf("[Brainf*ck] Generating executable...\n");
    system("gcc build/output.c -o build/output");
    printf("[Brainf*ck] Compiled successfuly!\n");
}

void run()
{
#ifdef _WIN32
    system("build\\output");
#else
    system("./build/output");
#endif
}

void usage()
{
    fprintf(stdout, "[Brainf*ck] Usage: ./bf <mode> <file>\n");
    fprintf(stdout, "[Brainf*ck] Modes: simulate, compile\n");
}

#define STREQUAL(s1, s2) (strcmp(s1, s2) == 0)

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage();
        return 1;
    }

    if (STREQUAL(argv[1], "run"))
        run();
    else
    {
        if (argc == 2)
        {
            usage();
            return 1;
        }

        char *input = NULL;
        size_t input_length = read_file(argv[2], &input);

        if (input_length == 0)
            return 1;

        printf("[Brainf*ck] Total characters: %d\n", input_length);

        if (STREQUAL(argv[1], "simulate"))
            simulate(input, input_length);
        else if (STREQUAL(argv[1], "compile"))
            compile(input, input_length);
        else
        {
            fprintf(stderr, "[Brainf*ck] Invalid mode!\n");
            return 1;
        }
    }

    return 0;
}
