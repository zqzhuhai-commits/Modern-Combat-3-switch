/* SPDX-License-Identifier: MIT.
 * ARMv8 AArch32 traps the old SWP instruction used by this native library.
 * Resume exactly that instruction with an atomic exchange, preserving CPSR.
 * Only active inside this hash-verified module's executable segment.
 */
#include "exc_handler.h"
#include <stdint.h>
static uint32_t text_start,text_end;
static unsigned swaps;
void mc3_swp_region(uint32_t start,uint32_t end){text_start=start;text_end=end;}
unsigned mc3_swp_count(void){return __atomic_load_n(&swaps,__ATOMIC_RELAXED);}
int port_exception_hook(uint32_t type,ThreadExceptionInfo32 *info,ThreadExceptionFrame32 *frame){
 if(!info||!frame || (type!=EXC_INSTRUCTION_ABORT&&type!=EXC_UNDEFINED_INSTRUCTION))return 0;
 if(text_end<=text_start || text_end-text_start<4)return 0;
 if(info->esr>>26 || (info->pstate&32) || info->pc<text_start || info->pc>text_end-4 || info->pc&3)return 0;
 uint32_t word=*(const uint32_t *)(uintptr_t)info->pc;
 if((word&0xffb00ff0u)!=0xe1000090u)return 0;
 unsigned rn=(word>>16)&15,rd=(word>>12)&15,rm=word&15;
 if(rn>=15||rd>=15||rm>=15||rn==rd||rn==rm)return 0;
 uint32_t *base=rt_exc_reg(info,frame,rn),*dest=rt_exc_reg(info,frame,rd),*src=rt_exc_reg(info,frame,rm);
 if(!base||!dest||!src)return 0;
 uint32_t address=*base,value=*src;int byte=(word>>22)&1;
 if((!byte&&(address&3)) || !address)return 0;
 MemoryInfo memory;uint32_t page;
 if(R_FAILED(svcQueryMemory(&memory,&page,address)) || (memory.perm&3)!=3)return 0;
 unsigned width=byte?1:4;
 if(address<memory.addr || (uint64_t)address+width>memory.addr+memory.size)return 0;
 uint32_t old=byte?__atomic_exchange_n((uint8_t *)(uintptr_t)address,(uint8_t)value,__ATOMIC_SEQ_CST)
                  :__atomic_exchange_n((uint32_t *)(uintptr_t)address,value,__ATOMIC_SEQ_CST);
 *dest=old;info->pc+=4;__atomic_fetch_add(&swaps,1,__ATOMIC_RELAXED);return 1;
}
