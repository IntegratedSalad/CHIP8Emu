#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "emu.h"

#define MAX_LINE_SIZE    128
#define MAX_TOKEN_SIZE   (MAX_LINE_SIZE / 2)
#define MAX_TCS_TESTFILE 256

typedef enum
{
    ERROR_NO_SPACE_AFTER_COMMA_ARRAY,
    ERROR_ARRAY_DOESNT_START_WITH_BRACKET,
    ERROR_NO_ENDTC_OR_EMPTY_LINE,
    ERROR_INVALID_TOKEN,
    ERROR_LINE_TO_LONG,
    ERROR_NO_ERROR
} Error_e;

Error_e err = ERROR_NO_ERROR;
int tcLine = 0;

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

// Holds data for one TC
typedef struct
{
    char     name[MAX_TOKEN_SIZE];
    Params_t paramsIn;
    Params_t paramsOut;
    Params_t paramsOutReference; // reference
    uint8_t  programMemory[MEMORY_SIZE]; // instructions
} Test_t;

bool parseTestFile(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out,
               Test_t** tcArray_p);

void executeAndVerifyAllTests(Test_t* tcArray_p[MAX_TCS_TESTFILE]);
float executeTest(Test_t* test_p);
bool verifyTest(const Test_t* const tp);
void cleanUpTestSuite(Test_t* tcArray_p[MAX_TCS_TESTFILE]);
void printError(void);

ssize_t readLineFromFd(int fd, char line[MAX_LINE_SIZE],
                       int* totalOffset, bool* isOk);
Token_e parseLine(const char line[MAX_LINE_SIZE],
                  char valueStr[MAX_TOKEN_SIZE]);
bool getByteArray(const char byteString[MAX_TOKEN_SIZE], uint8_t byteArray[NUM_OF_REGISTERS]);
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

    Test_t* testCaseArray[MAX_TCS_TESTFILE] = {0};
    bool testValid = parseTestFile(fd, &paramsIn, &paramsOut, testCaseArray);
    if (!testValid)
    {
        fprintf(stderr, "Found error in .yaml %s\n", fileName);
        printError();
        fprintf(stdout, "Test FAILED.\n");
        exit(-1);
    }

    executeAndVerifyAllTests(testCaseArray);
    cleanUpTestSuite(testCaseArray);

    return 0;
}

bool parseTestFile(int fd,
               const Params_t* const param_in,
               const Params_t* const param_out,
               Test_t** tcArray_p)
{
    // TODO: Trace line number
    bool isOk = true;
    bool isRegTypeInput = true;
    char line[MAX_LINE_SIZE];
    line[MAX_LINE_SIZE - 1] = '\0';

    char valueFromYaml[MAX_TOKEN_SIZE];
    ssize_t bytes_read = 0;
    int totalOffset = 0;

    uint8_t byteArray[NUM_OF_REGISTERS] = {0};

    int tcIdx = 0;
    Test_t* tc_p = NULL;

    bool isEndTc = false;
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
                    tc_p = malloc(sizeof(Test_t)); // TODO: remove this here
                    memcpy(tc_p->name, valueFromYaml, MAX_TOKEN_SIZE);
                    // TODO: allocate here new tc
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
                    isRegTypeInput = true;
                    break;
                }
                case TOKEN_PROGRAM:
                {
                    printf("PROGRAM! \n");
                    getByteArray(valueFromYaml, byteArray);
                    memcpy(tc_p->programMemory, byteArray, MEMORY_SIZE);
                    break;
                }
                case TOKEN_OUTPUT:
                {
                    printf("OUTPUT! \n");
                    isRegTypeInput = false;
                    break;
                }
                case TOKEN_REGS:
                {
                    printf("REGS! \n");
                    getByteArray(valueFromYaml, byteArray);
                    if (isRegTypeInput)
                    {
                        memcpy(tc_p->paramsIn.registerArray, byteArray, NUM_OF_REGISTERS);
                    } else
                    {
                        memcpy(tc_p->paramsOutReference.registerArray, byteArray, NUM_OF_REGISTERS);
                    }
                    break;
                }
                case TOKEN_END_TC:
                {
                    printf("END_TC! \n");
                    tcArray_p[tcIdx] = tc_p;
                    tcIdx++;
                    isEndTc = true;
                    break;
                }
                default:
                {
                    printf("INVALID! \n");
                    err = ERROR_INVALID_TOKEN;
                    return false;
                }
            }
            memset(line, 0, MAX_LINE_SIZE);
            memset(byteArray, 0, NUM_OF_REGISTERS);
            tcLine++;
        }
    }
    if (!isEndTc)
    {
        err = ERROR_NO_ENDTC_OR_EMPTY_LINE;
        isOk = isEndTc;
    }
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
        err = ERROR_LINE_TO_LONG;
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
    if (*line == '-')
    {
        return TOKEN_END_TC;
    }
    if (colon_p == NULL && comment_p != NULL && comment_p != line) return tok;
    if (comment_p == line) return TOKEN_COMMENT;

    char key[MAX_TOKEN_SIZE];
    memcpy(key, line, (size_t)(colon_p - line));
    memcpy(valueStr, colon_p + 2, (size_t)(end_p - colon_p - 1));
    key[(size_t)(colon_p - line)] = '\0';
    valueStr[(size_t)(end_p - colon_p)] = '\0';

    if (strcmp("name", key) == 0)
    {
        tok = TOKEN_NAME;
    } else if (strcmp("comment", key) == 0)
    {
        tok = TOKEN_COMMENT;
    } else if (strcmp("input", key) == 0)
    {
        tok = TOKEN_INPUT;
    } else if (strcmp("  program", key) == 0)
    {
        tok = TOKEN_PROGRAM;
    } else if (strcmp("output", key) == 0)
    {
        tok = TOKEN_OUTPUT;
    } else if (strcmp("  regs", key) == 0)
    {
        tok = TOKEN_REGS;
    }
    printf("key s: %s\n", key);
    printf("value s: %s\n", valueStr);
    return tok;
}

void executeAndVerifyAllTests(Test_t* tcArray_p[MAX_TCS_TESTFILE])
{
    int testCount = 0;
    int testPassed = 0;

    Test_t* tp;
    for (int tIdx = 0; tIdx < MAX_TCS_TESTFILE; tIdx++)
    {
        tp = tcArray_p[tIdx];
        if (tp == NULL) break;
        testCount++;
        printf("%s starting...\n", tp->name);
        const float timeElapsed = executeTest(tp);
        printf("tc %s took %fs\n", tp->name, timeElapsed);
        printf("Verifying...\n");
        if (verifyTest(tp))
        {
            testPassed++;
            printf("Test %s PASSED.\n", tp->name);
        } else
        {
            printf("Test %s FAILED.\n", tp->name);
        }
    }
    printf("%d/%d test cases passed.\n", testPassed, testCount);
}

float executeTest(Test_t* test_p)
{
    OPCodeData opcodeData;
    uint16_t currentInstructionCode = 0;
    Emulator_ExecutionHandler execHandler = NULL;
    uint8_t instructionType;

    Emulator* emu_p;
    Emulator_Init(&emu_p);

    Emulator_LoadProgramFromMemoryArray(emu_p, test_p->programMemory);

    clock_t start = clock();
    currentInstructionCode = Emulator_Fetch(emu_p);
    while (currentInstructionCode != 0)
    {
        instructionType = Emulator_Decode(emu_p, currentInstructionCode, &opcodeData);
        execHandler = Emulator_MapExecutionHandler(instructionType, &opcodeData);
        execHandler(&opcodeData, NULL, emu_p);
        currentInstructionCode = Emulator_Fetch(emu_p);
    }
    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    memcpy(test_p->paramsOut.registerArray, emu_p->registerArray, NUM_OF_REGISTERS);

    Emulator_DeInit(emu_p);
    return seconds;
}

bool verifyTest(const Test_t* const tp)
{
    const Params_t paramsOut = tp->paramsOut;
    const Params_t paramsOutReference = tp->paramsOutReference;
    for (int rIdx = 0; rIdx < NUM_OF_REGISTERS; rIdx++)
    {
        const uint8_t registerArrayOutVal = paramsOut.registerArray[rIdx];
        const uint8_t registerArrayOutReferenceVal = paramsOutReference.registerArray[rIdx];
        if (registerArrayOutVal != registerArrayOutReferenceVal)
        {
            return false;
        }
    }
    return true;
}

void cleanUpTestSuite(Test_t* tcArray_p[MAX_TCS_TESTFILE])
{
    for (int tIdx = 0; tIdx < MAX_TCS_TESTFILE; tIdx++)
    {
        free(tcArray_p[tIdx]);
    }
}

bool getByteArray(const char byteString[MAX_TOKEN_SIZE], uint8_t byteArray[NUM_OF_REGISTERS])
{
    if (*byteString != '[')
    {
        err = ERROR_ARRAY_DOESNT_START_WITH_BRACKET;
        return false;
    }
    bool isOk = true;
    char buffer[4];
    int i = 0;
    int byteArrayIdx = 0;

    ssize_t len = strlen(byteString);
    for (const char* p = byteString; i < len; p++, i++)
    {
        printf("%c\n ", *p);
        if (*p == '[')
        {
            buffer[0] = *(p+1);
            buffer[1] = *(p+2);
            buffer[2] = *(p+3);
            buffer[3] = *(p+4);
            byteArray[byteArrayIdx] = (uint8_t)strtol(buffer, NULL, 0);
            byteArrayIdx++;
            p += 4;
            i += 4;
        } else if (*p == ',')
        {
            if (*(p+1) != ' ')
            {
                err = ERROR_NO_SPACE_AFTER_COMMA_ARRAY;
                isOk = false;
                return isOk;
            }
            buffer[0] = *(p+2);
            buffer[1] = *(p+3);
            buffer[2] = *(p+4);
            buffer[3] = *(p+5);
            byteArray[byteArrayIdx] = (uint8_t)strtol(buffer, NULL, 0);
            byteArrayIdx++;
            p += 4;
            i += 4;
        }
        memset(buffer, 0, 4);
    }
    return isOk;
}

void printError(void)
{
    // TODO: Print yaml line of error
    switch (err)
    {
        case ERROR_ARRAY_DOESNT_START_WITH_BRACKET:
        {
            fprintf(stderr, "%s\n", "Array does not start with a bracket!");
            break;
        }
        case ERROR_INVALID_TOKEN:
        {
            fprintf(stderr, "%s\n", "Invalid token!");
            break;
        }
        case ERROR_NO_ENDTC_OR_EMPTY_LINE:
        {
            fprintf(stderr, "%s\n", "No empty line at the end of testsuite or '---' at the end of tc!");
            break;
        }
        case ERROR_NO_SPACE_AFTER_COMMA_ARRAY:
        {
            fprintf(stderr, "%s\n", "No space after comma in array!");
            break;
        }
        case ERROR_LINE_TO_LONG:
        {
            fprintf(stderr, "%s\n", "Line too long!");
            break;
        }
        default:
        {
            break;
        }
    }
}

/* TODO:
    1. Parse program (instructions)

*/