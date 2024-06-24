#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "cmdline.h"


uint32_t parse_cmdline(int argc, char* argv[], const cmdline_t* params, uint32_t paramCount, int startParam) {
    char parm[256] = { 0 };
    for (int i = startParam; i < argc; i++) {
        // copy to temp buffer and uppercase it
        strncpy(parm, argv[i], sizeof(parm)); _strupr(parm);
        char* p = parm;

        // strip separators
        while ((*p == '-') || (*p == '/')) p++;

        // get best matching option
        const cmdline_t* cmd = params; char* nextp = p;
        for (int j = 0; j < paramCount; j++, cmd++) {
            if ((cmd->longname != NULL) && (strstr(p, cmd->longname) != NULL)) {
                p += strlen(cmd->longname);
                break;
            }
            if (*p == cmd->shortname) {
                p++; break;
            }
        }
        if (cmd->parmPtr == NULL) {
            fprintf(stderr, "error: unknown parameter: %s\n", parm);
            return 1;
        }

        if (cmd->flags == CMD_FLAG_NONE) {
            *(uint32_t*)cmd->parmPtr = 1;
            continue;       // get next param
        }

        if (*p == 0) {
            if (cmd->flags == CMD_FLAG_BOOL) {
                *(uint32_t*)cmd->parmPtr = 1;
                continue;       // get next param
            }
            else {
                // oops - end of string
                i++; if (i == argc) {
                    fprintf(stderr, "error: incorrect parameter: %s\n", parm);
                    return 1;
                }
                strncpy(parm, argv[i], sizeof(parm)); _strupr(parm);
                p = parm;
            }
        }
        // else skip more separators
        while ((*p == '=') || (*p == ':')) p++;

        // finally extract value
        switch (cmd->flags & CMD_FLAG_MASK) {
        case CMD_FLAG_BOOL:
            if (strstr(p, "ON") || strstr(p, "1")) *(uint32_t*)cmd->parmPtr = 1;
            else if (strstr(p, "OFF") || strstr(p, "0")) *(uint32_t*)cmd->parmPtr = 0;
            else {
                fprintf(stderr, "error: incorrect parameter: %s\n", p);
                return 1;
            }
            break;
        case CMD_FLAG_INT:
            if (sscanf(p, "%d", (uint32_t*)cmd->parmPtr) != 1) {
                fprintf(stderr, "error: incorrect parameter: %s\n", p);
                return 1;
            }
        case CMD_FLAG_HEX:
            if (sscanf(p, "%x", (uint32_t*)cmd->parmPtr) != 1) {
                fprintf(stderr, "error: incorrect parameter: %s\n", p);
                return 1;
            }
        case CMD_FLAG_STRING:
            strncpy((char*)cmd->parmPtr, p, cmd->parmLength);
            break;
        default:
            fprintf(stderr, "error: undefined parameter type %d for %c(\"%s\")!\n", cmd->flags, cmd->shortname, cmd->longname);
            return 1;
        }
    }

    return 0;
}
