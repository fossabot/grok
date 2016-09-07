﻿/*!
 * \brief   The file contains configuration module interface
 * \author  \verbatim
            Created by: Alexander Egorov
            \endverbatim
 * \date    \verbatim
            Creation date: 2015-09-01
            \endverbatim
 * Copyright: (c) Alexander Egorov 2015
 */


#ifndef GROK_CONFIGURATION_H_
#define GROK_CONFIGURATION_H_

typedef struct configuration_ctx_t {
    void (*on_string)(struct arg_file* files, char* const macro, char* const str);
    void (*on_file)(struct arg_file* files, char* const macro, char* const path);
    int argc;
    char** argv;
} configuration_ctx_t;

void conf_configure_app(configuration_ctx_t* ctx);

#endif // GROK_CONFIGURATION_H_
