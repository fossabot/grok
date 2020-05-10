﻿// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*!
 * \brief   The file contains configuration module implementation
 * \author  \verbatim
            Created by: Alexander Egorov
            \endverbatim
 * \date    \verbatim
            Creation date: 2015-09-01
            \endverbatim
 * Copyright: (c) Alexander Egorov 2015-2020
 */

#include "argtable3.h"
#include "targetver.h"
#include "configuration.h"
#include "lib.h"

#define OPT_PATT_SHORT "p"
#define OPT_PATT_LONG "patterns"
#define OPT_PATT_DESCR "one or more pattern files. You can also use wildcards like path\\*.patterns"

#define OPT_HELP_SHORT "h"
#define OPT_HELP_LONG "help"
#define OPT_HELP_DESCR "print this help and exit"

#define OPT_NOGREP_SHORT "i"
#define OPT_NOGREP_LONG "info"
#define OPT_NOGREP_DESCR "dont work like grep i.e. output matched string with additional info"

#define OPT_MACRO_SHORT "m"
#define OPT_MACRO_LONG "macro"
#define OPT_MACRO_DESCR "pattern macros to build regexp"

#define OPT_STR_SHORT "s"
#define OPT_STR_LONG "string"
#define OPT_STR_DESCR "string to match"

#define OPT_FILE_SHORT "f"
#define OPT_FILE_LONG "file"
#define OPT_FILE_DESCR "full path to file to read data from"

 /*
    conf_ - public members
    prconf_ - private members
 */

void prconf_print_copyright(void);
void prconf_print_syntax(void* argtable, void* argtableS, void* argtableF);

void conf_configure_app(configuration_ctx_t* ctx) {
    struct arg_lit* help = arg_lit0(OPT_HELP_SHORT, OPT_HELP_LONG, OPT_HELP_DESCR);
    struct arg_lit* helpF = arg_lit0(OPT_HELP_SHORT, OPT_HELP_LONG, OPT_HELP_DESCR);
    struct arg_lit* helpS = arg_lit0(OPT_HELP_SHORT, OPT_HELP_LONG, OPT_HELP_DESCR);
    
    struct arg_lit* info = arg_lit0(OPT_NOGREP_SHORT, OPT_NOGREP_LONG, OPT_NOGREP_DESCR);
    struct arg_lit* infoF = arg_lit0(OPT_NOGREP_SHORT, OPT_NOGREP_LONG, OPT_NOGREP_DESCR);
    struct arg_lit* infoS = arg_lit0(OPT_NOGREP_SHORT, OPT_NOGREP_LONG, OPT_NOGREP_DESCR);

    struct arg_str* string = arg_str1(OPT_STR_SHORT, OPT_STR_LONG, NULL, OPT_STR_DESCR);
    struct arg_str* stringG = arg_str0(OPT_STR_SHORT, OPT_STR_LONG, NULL, OPT_STR_DESCR);
    struct arg_file* file = arg_file1(OPT_FILE_SHORT, OPT_FILE_LONG, NULL, OPT_FILE_DESCR);
    struct arg_file* fileG = arg_file0(OPT_FILE_SHORT, OPT_FILE_LONG, NULL, OPT_FILE_DESCR);

    struct arg_str* macro = arg_str1(OPT_MACRO_SHORT, OPT_MACRO_LONG, NULL, OPT_MACRO_DESCR);
    struct arg_str* macroS = arg_str1(OPT_MACRO_SHORT, OPT_MACRO_LONG, NULL, OPT_MACRO_DESCR);
    struct arg_str* macroF = arg_str1(OPT_MACRO_SHORT, OPT_MACRO_LONG, NULL, OPT_MACRO_DESCR);

    struct arg_file* patterns = arg_filen(OPT_PATT_SHORT, OPT_PATT_LONG, NULL, 0, ctx->argc + 2, OPT_PATT_DESCR);
    struct arg_file* patternsS = arg_filen(OPT_PATT_SHORT, OPT_PATT_LONG, NULL, 0, ctx->argc + 2, OPT_PATT_DESCR);
    struct arg_file* patternsF = arg_filen(OPT_PATT_SHORT, OPT_PATT_LONG, NULL, 0, ctx->argc + 2, OPT_PATT_DESCR);

    struct arg_end* end = arg_end(10);
    struct arg_end* endF = arg_end(10);
    struct arg_end* endS = arg_end(10);

    void* argtable[] = {help, info, stringG, fileG, macro, patterns, end};
    void* argtableF[] = {helpF, infoF, file, macroF, patternsF, endF};
    void* argtableS[] = {helpS, infoS, string, macroS, patternsS, endS};

    if(arg_nullcheck(argtable) != 0 || arg_nullcheck(argtableF) != 0 || arg_nullcheck(argtableS) != 0) {
        prconf_print_syntax(argtable, argtableS, argtableF);
        goto cleanup;
    }

    int nerrors = arg_parse(ctx->argc, (char**)ctx->argv, argtable);
    int nerrorsF = arg_parse(ctx->argc, (char**)ctx->argv, argtableF);
    int nerrorsS = arg_parse(ctx->argc, (char**)ctx->argv, argtableS);

    if(nerrors > 0 || help->count > 0) {
        prconf_print_syntax(argtable, argtableS, argtableF);
        if(help->count == 0 && ctx->argc > 1) {
            arg_print_errors(stdout, end, PROGRAM_NAME);
        }
        goto cleanup;
    }

    if(nerrorsS == 0) {
        ctx->on_string(patternsS, macroS->sval[0], string->sval[0], infoS->count > 0);
    }
    else if(nerrorsF == 0) {
        ctx->on_file(patternsS, macroF->sval[0], file->filename[0], infoF->count > 0);
    }
    else {
        prconf_print_syntax(argtable, argtableS, argtableF);
        arg_print_errors(stdout, endF, PROGRAM_NAME);
        arg_print_errors(stdout, endS, PROGRAM_NAME);
    }

cleanup:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    arg_freetable(argtableS, sizeof(argtableS) / sizeof(argtableS[0]));
    arg_freetable(argtableF, sizeof(argtableF) / sizeof(argtableF[0]));
}

void prconf_print_copyright(void) {
    lib_printf(COPYRIGHT_FMT, APP_NAME);
}

void prconf_print_syntax(void* argtable, void* argtableS, void* argtableF) {
    prconf_print_copyright();

    lib_printf(PROG_EXE);
    arg_print_syntax(stdout, argtableS, NEW_LINE NEW_LINE);

    lib_printf(PROG_EXE);
    arg_print_syntax(stdout, argtableF, NEW_LINE NEW_LINE);

    arg_print_glossary_gnu(stdout, argtable);
}
