#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "emu.h"

#define MAX_LINE_SIZE  128
#define MAX_TOKEN_SIZE (MAX_LINE_SIZE / 2)

typedef enum
{
    TOKEN_NAME,
    TOKEN_COMMENT,
    TOKEN_INPUT,
    TOKEN_PROGRAM,
    TOKEN_OUTPUT,
    TOKEN_REGS,
    TOKEN_END_TC,
    TOKEN_INVALID
} Token_e;

typedef struct
{
    uint8_t registerArray[NUM_OF_REGISTERS];
} Params_t;

bool verify(const Params_t* const param_in,
            const Params_t* const param_out);
bool parseTest(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out);

ssize_t readLineFromFd(int fd, char line[MAX_LINE_SIZE],
                       int* totalOffset, bool* isOk);
Token_e parseLine(const char line[MAX_LINE_SIZE],
                  char valueStr[MAX_TOKEN_SIZE]);
int main(int argc, char** argv)
{
    Params_t paramsOut;
    Params_t paramsIn;

    if (argc < 2)
    {
        fprintf(stderr, "%s\n", "Provide a filename...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }
    if (strlen(argv[0]) == 0)
    {
        fprintf(stderr, "%s\n", "Provide a non-empty filename...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }

    const char* fileName = argv[1];
    fprintf(stdout, "%s!\n", fileName);

    int fd = open(fileName, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "%s\n", "Couldn't open the file...");
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }

    bool testValid = parseTest(fd, &paramsIn, &paramsOut);
    if (!testValid)
    {
        fprintf(stderr, "Found error in .yaml %s\n", fileName);
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }
    if (verify(&paramsIn, &paramsOut))
    {
        fprintf(stdout, "Test PASSED.\n");
    } else
    {
        fprintf(stdout, "Test FAILED.\n");
    }
    return 0;
}

bool parseTest(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out)
{
    bool isOk = true;
    char line[MAX_LINE_SIZE];
    char valueFromYaml[MAX_TOKEN_SIZE];
    line[MAX_LINE_SIZE - 1] = '\0';
    ssize_t bytes_read = 0;
    int totalOffset = 0;
    while ((bytes_read = readLineFromFd(fd, line, &totalOffset, &isOk)) > 0)
    {
        if (strnlen(line, MAX_LINE_SIZE) > 0)
        {
            fprintf(stdout, "Line: %s\n", line);
            Token_e tok = parseLine(line, valueFromYaml);
            switch (tok)
            {
                case TOKEN_NAME:
                {
                    printf("NAME! \n");
                    break;
                }
                case TOKEN_COMMENT:
                {
                    printf("COMMENT! \n");
                    break;
                }
                case TOKEN_INPUT:
                {
                    printf("INPUT! \n");
                    break;
                }
                case TOKEN_PROGRAM:
                {
                    printf("PROGRAM! \n");
                    break;
                }
                case TOKEN_OUTPUT:
                {
                    printf("OUTPUT! \n");
                    break;
                }
                case TOKEN_REGS:
                {
                    printf("REGS! \n");
                    break;
                }
                case TOKEN_END_TC:
                {
                    printf("END_TC! \n");
                    break;
                }
                default:
                {
                    printf("INVALID! \n");
                    isOk = false;
                    return isOk;
                }
            }
            memset(line, 0, MAX_LINE_SIZE);
        }
    }
    return isOk;
}

bool verify(const Params_t* const param_in,
            const Params_t* const param_out)
{
    bool isOk = true;
    return isOk;
}

ssize_t readLineFromFd(int fd, char line[MAX_LINE_SIZE],
                       int* totalOffset, bool* isOk)
{
    *isOk = true;
    char buff[MAX_LINE_SIZE];
    ssize_t bytes_read = read(fd, buff, MAX_LINE_SIZE - 2);
    if (bytes_read <= 0)
    {
        return bytes_read;
    }

    int i = 0;
    char* c;
    for (c = buff;
         *c != '\n' && *c != 0 && i <= MAX_LINE_SIZE - 2;
         c++, i++);
    if (i >= MAX_LINE_SIZE - 2)
    {
        printf("Line: %s\n", buff);
        fprintf(stderr, "Line too long! %d\nMAX: %d\n", i, MAX_LINE_SIZE - 2);
        *isOk = false;
        return 0;
    }
    if (buff[i] == '\n')
    {
        line[i] = '\0';
        memcpy(line, buff, i);
        // printf("totalOffset: %d\n", *totalOffset);
        // printf("i: %d\n", i);

        // i + 1 can be another '\n', (if there are empty lines):
        // we have to increment until next char is not '\n
        i++;
        for (char* _c = c+1; *_c == '\n' && *_c != 0; _c++, i++);
        lseek(fd, *(totalOffset) + i, SEEK_SET);
    }
    *totalOffset += i;
    return bytes_read;
}

Token_e parseLine(const char line[MAX_LINE_SIZE], char valueStr[MAX_TOKEN_SIZE])
{
    Token_e tok = TOKEN_INVALID;
    const char* end_p = strchr(line, '\0');
    const char* colon_p = strchr(line, ':');
    const char* comment_p = strchr(line, '#');
    if (colon_p == NULL && comment_p != NULL && comment_p != line) return tok;
    if (comment_p == line) return TOKEN_COMMENT;

    char key[MAX_TOKEN_SIZE];
    memcpy(key, line, (size_t)(colon_p - line));
    memcpy(valueStr, colon_p + 2, (size_t)(end_p - colon_p - 1));
    key[(size_t)(colon_p - line)] = '\0';
    valueStr[(size_t)(end_p - colon_p)] = '\0';

    if (strcmp("name", key))
    {
        tok = TOKEN_NAME;
    } else if (strcmp("comment", key))
    {
        tok = TOKEN_COMMENT;
    } else if (strcmp("input", key))
    {
        tok = TOKEN_INPUT;
    } else if (strcmp("program", key))
    {
        tok = TOKEN_PROGRAM;
    } else if (strcmp("output", key))
    {
        tok = TOKEN_OUTPUT;
    } else if (strcmp("regs", key))
    {
        tok = TOKEN_REGS;
    } else if (strcmp("---", key))
    {
        tok = TOKEN_END_TC;
    }
    printf("key s: %s\n", key);
    printf("value s: %s\n", valueStr);
    return tok;
}