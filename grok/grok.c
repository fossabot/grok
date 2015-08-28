/*!
 * \brief   The file contains compiler driver
 * \author  \verbatim
            Created by: Alexander Egorov
            \endverbatim
 * \date    \verbatim
            Creation date: 2015-07-21
            \endverbatim
 * Copyright: (c) Alexander Egorov 2015
 */


#define PCRE2_CODE_UNIT_WIDTH 8

#include "targetver.h"

#include <stdio.h>
#include <locale.h>
#include "apr.h"
#include "grok.tab.h"
#include "frontend.h"
#include <apr_errno.h>
#include <apr_general.h>
#include <apr_strings.h>
#include "argtable2.h"
#include "../pcre/pcre2.h"

#define OPT_F_SHORT "p"
#define OPT_F_LONG "patterns"
#define OPT_F_DESCR "one or more pattern files"


// Forwards
extern void yyrestart(FILE* input_file);
BOOL match_re(char* pattern, char* subject);
void* pcre_alloc(size_t, void*);
void pcre_free(void*, void*);
char* create_pattern(apr_array_header_t* parts);

pcre2_general_context* pcre_context = NULL;
apr_pool_t* main_pool;

void Parse();


int main(int argc, char* argv[]) {
    errno_t error = 0;
    apr_status_t status = APR_SUCCESS;

    struct arg_str* string = arg_str0("s", "string", NULL, "string to match");
    struct arg_str* macro = arg_str0("m", "macro", NULL, "pattern macros to build regexp");
    struct arg_file* files = arg_filen(OPT_F_SHORT, OPT_F_LONG, NULL, 1, argc + 2, OPT_F_DESCR);
    struct arg_end* end = arg_end(10);
    int nerrors = 0;
    int i = 0;

    void* argtable[] = {string, macro, files, end};

    setlocale(LC_ALL, ".ACP");
    setlocale(LC_NUMERIC, "C");

    status = apr_app_initialize(&argc, &argv, NULL);
    if(status != APR_SUCCESS) {
        CrtPrintf("Couldn't initialize APR");
        return EXIT_FAILURE;
    }

    atexit(apr_terminate);

    apr_pool_create(&main_pool, NULL);
    pcre_context = pcre2_general_context_create(&pcre_alloc, &pcre_free, NULL);
    fend_init(main_pool);

    // read from stdin
    if(argc < 2) {
        Parse();
        goto cleanup;
    }

    if(arg_nullcheck(argtable) != 0) {
        arg_print_syntax(stdout, argtable, NEW_LINE NEW_LINE);
        arg_print_glossary_gnu(stdout, argtable);
        goto cleanup;
    }

    nerrors = arg_parse(argc, argv, argtable);

    if(nerrors > 0) {
        arg_print_syntax(stdout, argtable, NEW_LINE NEW_LINE);
        arg_print_glossary_gnu(stdout, argtable);
        goto cleanup;
    }

    for(; i < files->count; i++) {
        FILE* f = NULL;
        const char* p = files->filename[i];
        error = fopen_s(&f, p, "r");
        if(error) {
            perror(argv[1]);
            goto cleanup;
        }
        yyrestart(f);
        Parse();
        fclose(f);
    }

    if(string->count > 0 && macro->count > 0) {
        apr_array_header_t* parts = fend_get_pattern(macro->sval[0]);
        const char* pattern = create_pattern(parts);
        BOOL r = match_re(pattern, string->sval[0]);
        CrtPrintf("string: %s | match: %s | pattern: %s\n", string->sval[0], r > 0 ? "TRUE" : "FALSE", macro->sval[0]);
    }

cleanup:
    pcre2_general_context_free(pcre_context);
    apr_pool_destroy(main_pool);
    return 0;
}

void Parse() {
    if(yyparse()) {
        CrtPrintf("Parse failed\n");
    }
}

char* create_pattern(apr_array_header_t* parts) {
    char* result = "";
    for (int i = 0; i < parts->nelts; i++) {
        Info_t* info = ((Info_t**)parts->elts)[i];
        result = apr_pstrcat(main_pool, result, info->Info, NULL);
    }
    return result;
}

BOOL match_re(char* pattern, char* subject) {
    int errornumber = 0;
    size_t erroroffset = 0;

    pcre2_code* re = pcre2_compile(
        pattern, /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        0, /* default options */
        &errornumber, /* for error number */
        &erroroffset, /* for error offset */
        NULL); /* use default compile context */

    if(re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        CrtPrintf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        return FALSE;
    }
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);

    int flags = PCRE2_NOTEMPTY;
    if(!strstr(subject, "^")) {
        flags |= PCRE2_NOTBOL;
    }
    if(!strstr(subject, "$")) {
        flags |= PCRE2_NOTEOL;
    }

    int rc = pcre2_match(
        re, /* the compiled pattern */
        subject, /* the subject string */
        strlen(subject), /* the length of the subject */
        0, /* start at offset 0 in the subject */
        flags,
        match_data, /* block for storing the result */
        NULL); /* use default match context */
    return rc >= 0;
}

void* pcre_alloc(size_t size, void* memory_data) {
    return apr_palloc(main_pool, size);
}

void pcre_free(void* p1, void* p2) {}
