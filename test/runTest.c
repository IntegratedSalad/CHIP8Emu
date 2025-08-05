#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "emu.h"

#define N_BYTES 64

typedef struct
{
    uint8_t registerArray[NUM_OF_REGISTERS];
} Params_t;

bool verify(const Params_t* const param_in,
            const Params_t* const param_out);
void parseTest(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out);
int main(int argc, char** argv)
{
    Params_t paramsOut;
    Params_t paramsIn;

    if (argc < 2)
    {
        fprintf(stderr, "%s\n", "Provide a filename...");
        exit(-1);
    }
    if (strlen(argv[0]) == 0)
    {
        fprintf(stderr, "%s\n", "Provide a non-empty filename...");
        exit(-1);
    }

    const char* fileName = argv[0];
    fprintf(stdout, "%s!\n", fileName);

    int fd = open(fileName, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "%s\n", "Couldn't open the file...")
        exit(-1);
    }

    bool testValid = parseTest(fd, &paramsIn, &paramsOut);
    if (!testValid)
    {
        fprintf(stderr, "Found error in .yaml %s");
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
    char buff[N_BYTES];
    ssize_t bytes_read = 0;
    while ((bytes_read = read(fd, buff, N_BYTES)) > 0)
    {
        fprintf("%s\n", buff)
    }
    return isOk;
}

bool verify(const Params_t* const param_in,
            const Params_t* const param_out)
{
    bool isOk = true;
    return isOk;
}