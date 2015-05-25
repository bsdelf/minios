/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)tss.h	5.4 (Berkeley) 1/18/91
 * $FreeBSD: stable/9/sys/i386/include/tss.h 128019 2004-04-07 20:46:16Z imp $
 */

#include <types.h>

/*
 * Intel 386 Context Data Type
 */
#ifndef _MACHINE_TSS_H_
#define _MACHINE_TSS_H_ 1

#pragma pack(push, 1)
typedef struct i386tss {
	uint32	link;	/* actually 16 bits: top 16 bits must be zero */
	uint32	esp0; 	/* kernel stack pointer privilege level 0 */
	uint32	ss0;	/* actually 16 bits: top 16 bits must be zero */
	uint32	esp1; 	/* kernel stack pointer privilege level 1 */
	uint32	ss1;	/* actually 16 bits: top 16 bits must be zero */
	uint32	esp2; 	/* kernel stack pointer privilege level 2 */
	uint32	ss2;	/* actually 16 bits: top 16 bits must be zero */
	uint32	cr3; 	/* page table directory */
	uint32	eip; 	/* program counter */
	uint32	eflags; 	/* program status longword */
	uint32	eax;
	uint32	ecx;
	uint32	edx;
	uint32	ebx;
	uint32	esp; 	/* user stack pointer */
	uint32	ebp; 	/* user frame pointer */
	uint32	esi;
	uint32	edi;
	uint32	es;		/* actually 16 bits: top 16 bits must be zero */
	uint32	cs;		/* actually 16 bits: top 16 bits must be zero */
	uint32	ss;		/* actually 16 bits: top 16 bits must be zero */
	uint32	ds;		/* actually 16 bits: top 16 bits must be zero */
	uint32	fs;		/* actually 16 bits: top 16 bits must be zero */
	uint32	gs;		/* actually 16 bits: top 16 bits must be zero */
	uint32	ldt;	/* actually 16 bits: top 16 bits must be zero */
	uint32	ioopt;	/* options & io offset bitmap: currently zero */
				/* XXX unimplemented .. i/o permission bitmap */
} i386tss;
#pragma pack(pop)

#endif /* _MACHINE_TSS_H_ */
