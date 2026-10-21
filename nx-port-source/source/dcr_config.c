/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include "rt_cfg.h"
static const CfgOpt opts[]={
 CFG_ROW_BOOST(CFG_HELP_BOOST,NULL),
 CFG_ROW_GL_SELFTEST(NULL),
 CFG_ROW_BOOT_LOG(CFG_HELP_BOOT_LOG,NULL),
 CFG_ROW_LOG_JNI(CFG_HELP_LOG_JNI,NULL)
};
static const CfgTable table={.opts=opts,.nopts=CFG_COUNT(opts),.version=2};
void dcr_config_load(void){rt_config_load(&table);}
