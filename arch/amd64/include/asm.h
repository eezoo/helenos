/*
 * Copyright (C) 2005 Jakub Jermar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __amd64_ASM_H__
#define __amd64_ASM_H__

#include <arch/types.h>
#include <config.h>


void asm_delay_loop(__u32 t);
void asm_fake_loop(__u32 t);

/** Return base address of current stack.
 *
 * Return the base address of the current stack.
 * The stack is assumed to be STACK_SIZE bytes long.
 * The stack must start on page boundary.
 */
static inline __address get_stack_base(void)
{
	__address v;
	
	__asm__ volatile ("andq %%rsp, %0\n" : "=r" (v) : "0" (~((__u64)STACK_SIZE-1)));
	
	return v;
}

static inline void cpu_sleep(void) { __asm__ volatile ("hlt\n"); };
static inline void cpu_halt(void) { __asm__ volatile ("hlt\n"); };


static inline __u8 inb(__u16 port) 
{
	__u8 out;

	__asm__ volatile (
		"mov %1, %%dx\n"
		"inb %%dx,%%al\n"
		"mov %%al, %0\n"
		:"=m"(out)
		:"m"(port)
		:"%rdx","%rax"
		);
	return out;
}

static inline __u8 outb(__u16 port,__u8 b) 
{
	__asm__ volatile (
		"mov %0,%%dx\n"
		"mov %1,%%al\n"
		"outb %%al,%%dx\n"
		:
		:"m"( port), "m" (b)
		:"%rdx","%rax"
		);
}

/** Enable interrupts.
 *
 * Enable interrupts and return previous
 * value of EFLAGS.
 *
 * @return Old interrupt priority level.
 */
static inline ipl_t interrupts_enable(void) {
	ipl_t v;
	__asm__ volatile (
		"pushfq\n"
		"popq %0\n"
		"sti\n"
		: "=r" (v)
	);
	return v;
}

/** Disable interrupts.
 *
 * Disable interrupts and return previous
 * value of EFLAGS.
 *
 * @return Old interrupt priority level.
 */
static inline ipl_t interrupts_disable(void) {
	ipl_t v;
	__asm__ volatile (
		"pushfq\n"
		"popq %0\n"
		"cli\n"
		: "=r" (v)
		);
	return v;
}

/** Restore interrupt priority level.
 *
 * Restore EFLAGS.
 *
 * @param ipl Saved interrupt priority level.
 */
static inline void interrupts_restore(ipl_t ipl) {
	__asm__ volatile (
		"pushq %0\n"
		"popfq\n"
		: : "r" (ipl)
		);
}

/** Return interrupt priority level.
 *
 * Return EFLAFS.
 *
 * @return Current interrupt priority level.
 */
static inline ipl_t interrupts_read(void) {
	ipl_t v;
	__asm__ volatile (
		"pushfq\n"
		"popq %0\n"
		: "=r" (v)
	);
	return v;
}

/** Read CR0
 *
 * Return value in CR0
 *
 * @return Value read.
 */
static inline __u64 read_cr0(void) 
{ 
	__u64 v; 
	__asm__ volatile ("movq %%cr0,%0\n" : "=r" (v)); 
	return v; 
}

/** Read CR2
 *
 * Return value in CR2
 *
 * @return Value read.
 */
static inline __u64 read_cr2(void) 
{ 
	__u64 v; 
	__asm__ volatile ("movq %%cr2,%0\n" : "=r" (v)); 
	return v; 
}

/** Write CR3
 *
 * Write value to CR3.
 *
 * @param v Value to be written.
 */
static inline void write_cr3(__u64 v) 
{ 
	__asm__ volatile ("movq %0,%%cr3\n" : : "r" (v)); 
}

/** Read CR3
 *
 * Return value in CR3
 *
 * @return Value read.
 */
static inline __u64 read_cr3(void) 
{ 
	__u64 v;
	__asm__ volatile ("movq %%cr3,%0" : "=r" (v)); 
	return v; 
}


/** Enable local APIC
 *
 * Enable local APIC in MSR.
 */
static inline void enable_l_apic_in_msr()
{
	__asm__ volatile (
		"movl $0x1b, %%ecx\n"
		"rdmsr\n"
		"orl $(1<<11),%%eax\n"
		"orl $(0xfee00000),%%eax\n"
		"wrmsr\n"
		:
		:
		:"%eax","%ecx","%edx"
		);
}

static inline __address * get_ip() 
{
	__address *ip;

	__asm__ volatile (
		"mov %%rip, %0"
		: "=r" (ip)
		);
	return ip;
}


extern size_t interrupt_handler_size;
extern void interrupt_handlers(void);

#endif
