#include "args.h"
#include <string.h>
#include <time.h>
OptionsResult args_parse(Options *options, int count, const char *arguments[])
{
    OptionsResult ret = {
        .argno = 0,
        .otype = OT_NONE,
        .oerror = OE_SUCCESS,
    };
    options->width = 320;
    options->height = 240;
    options->seed = time(NULL);
    options->threads = 1;
    if (count == 0){
        ret.otype = OT_OUTPUT_FILE;
        ret.oerror = OE_SWITCH_REQUIRES_PARAMETER;
    }    
    int type = 0;
    char parse[4];
    int i;
    for (i = 0; i < count; i++) {
        sscanf(arguments[i], "%3s", parse);
        if (i == count-1) {
            strcpy(parse, "-e");
        }
        switch (parse[1]) {
            int temp;
            char temps[256];
            case 'd':
                if (strlen(arguments[i+1]) > 4) {
                    ret.argno = i+1;
                    ret.otype = OT_OUTPUT_TYPE;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                sscanf(arguments[i+1], "%4s", temps);
                strcpy(options->output_type, temps);
                i++;
                type = 1;
                break;
            case 'w':
                sscanf(arguments[i+1], "%d", &temp);
                if (temp <= MIN_WIDTH || temp >= MAX_WIDTH) {
                    ret.argno = i+1;
                    ret.otype = OT_WIDTH;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                options->width = temp;
                i++;
                break;
            case 'h':
                sscanf(arguments[i+1], "%d", &temp);
                if (temp <= MIN_HEIGHT || temp >= MAX_HEIGHT) {
                    ret.argno = i+1;
                    ret.otype = OT_HEIGHT;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                options->height = temp;
                i++;
                break;
            case 's':
                sscanf(arguments[i+1], "%d", &temp);
                if (temp <= MIN_SEED || temp >= MAX_SEED) {
                    ret.argno = i;
                    ret.otype = OT_SEED;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                options->seed = temp;
                i++;
                break;
            case 't':
                sscanf(arguments[i+1], "%d", &temp);
                if (temp <= MIN_THREADS || temp >= MAX_THREADS) {
                    ret.argno = i+1;
                    ret.otype = OT_THREADS;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                options->threads = temp;
                i++;
                break;
            case 'e':
                sscanf(arguments[count-1], "%256s", temps);
                if (count%2 != 1 && type == 0) {
                    ret.argno = 0;
                    ret.otype = OT_OUTPUT_TYPE;
                    ret.oerror = OE_SWITCH_REQUIRES_PARAMETER;
                    return ret;
                }
                if (strlen(arguments[count-1]) > 256) {
                    ret.argno = count-1;
                    ret.otype = OT_OUTPUT_FILE;
                    ret.oerror = OE_INVALID_VALUE_FOR_PARAMETER;
                    return ret;
                }
                if (type != 1) {
                    char temp2[4];
                    strncpy(temp2, temps+(strlen(temps)-3), 3);
                    temp2[3] = '\0';
                    strcpy(options->output_type, temp2);
                }
                
                strcpy(options->output_file, temps);
                break;
            default:
                ret.argno = i;
                ret.otype = OT_NONE;
                ret.oerror = OE_UNKNOWN_SWITCH;
                return ret;
        }
    }
    
    return ret;
}
