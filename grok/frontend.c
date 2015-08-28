﻿/*!
 * \brief   The file contains frontend implementation
 * \author  \verbatim
            Created by: Alexander Egorov
            \endverbatim
 * \date    \verbatim
            Creation date: 2015-07-21
            \endverbatim
 * Copyright: (c) Alexander Egorov 2015
 */


#include <stdio.h>
#include <stdlib.h>
#include  "frontend.h"
#include "apr.h"
#include "apr_tables.h"
#include "apr_pools.h"
#include "apr_strings.h"
#include "apr_hash.h"

#define ARRAY_INIT_SZ   256

// Forwards
void app_part(char*, Part_t);

apr_pool_t* fend_pool = NULL;
apr_hash_t* fend_definition = NULL;
apr_array_header_t* fend_composition = NULL;

void fend_init(apr_pool_t* p) {
    fend_pool = p;
    fend_definition = apr_hash_make(fend_pool);
}

void fend_on_definition() {
    fend_composition = apr_array_make(fend_pool, ARRAY_INIT_SZ, sizeof(Info_t*));
}

void fend_on_definition_end(char* key) {
    apr_array_header_t* parts = apr_array_make(fend_pool, fend_composition->nelts, sizeof(Info_t*));
    for (int i = 0; i < fend_composition->nelts; i++) {
        *(Info_t**)apr_array_push(parts) = ((Info_t**)fend_composition->elts)[i];
    }
    apr_hash_set(fend_definition, (const char*)key, APR_HASH_KEY_STRING, parts);
}

void fend_on_literal(char* str) {
    app_part(str, PartLiteral);
}

void fend_on_grok(char* str) {
    app_part(str, PartReference);
}

void app_part(char* data, Part_t type) {
    Info_t* result = (Info_t*)apr_pcalloc(fend_pool, sizeof(Info_t));
    result->Type = type;
    result->Info = data;
    *(Info_t**)apr_array_push(fend_composition) = result;
}

apr_array_header_t* fend_get_pattern(char* def) {
    apr_array_header_t* parts = apr_hash_get(fend_definition, (const char*)def, APR_HASH_KEY_STRING);
    return parts;
}

char* fend_strdup(char* str) {
    return apr_pstrdup(fend_pool, str);
}
