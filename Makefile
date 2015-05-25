AS			= yasm
CC			= clang
LD			= ld
LDFLAGS		= -melf_i386_fbsd
ASBFLAGS	= -w
ASEFLAGS	= -w -f elf32
ASFLAGS		= -felf
CFLAGS		= -std=c99 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -m32 -O1
WORKDIR		= work

PA_INFO			= 0x500				# up to 0x7c00, 30KB, BUT be careful about boot's stack
PA_BOOT			= 0x7c00			# 512 bytes
PA_LOADER		= 0x7e00			# up to 0x7ffff, 480.5KB
PA_KERNEL		= 0x100000			# should be at least PA_LOADER+wc(loader+kernel)
SIZE_KSTACK		= 0x2000			# 4KB guard included

VA_USER		= 0x400000
VA_KERNEL	= 0xc0400000

.PHONY: all clean

all: floppy.img

clean:
	rm -rf ${WORKDIR}

workdir: Makefile
	mkdir -p ${WORKDIR}

# bootloader
${WORKDIR}/boot.bin: boot/boot.s boot/adef.inc Makefile
	$(AS) $(ASBFLAGS) -o ${WORKDIR}/boot.bin boot/boot.s\
		-D PA_INFO=${PA_INFO} -D PA_BOOT=${PA_BOOT} -D PA_LOADER=${PA_LOADER} 

${WORKDIR}/loader.bin: boot/loader.s boot/adef.inc Makefile
	$(AS) $(ASBFLAGS) -o ${WORKDIR}/loader.bin boot/loader.s\
		-D PA_INFO=${PA_INFO} -D PA_LOADER=${PA_LOADER}\
		-D PA_KERNEL=${PA_KERNEL} -D VA_KERNEL=${VA_KERNEL} -D SIZE_KSTACK=${SIZE_KSTACK}

# kernel
${WORKDIR}/kernel.elf: kernel/* libk/*
	eb cc=$(CC)\
		workdir="${WORKDIR}" \
    	flag="${CFLAGS} -c -Ilibk/" \
		ld="ld" ldflag="${LDFLAGS} -Ttext $(VA_KERNEL) -e start" \
		as="$(AS)" asflag="${ASFLAGS}" out=kernel.elf libk/* kernel/*

# server
${WORKDIR}/zombie: server/zombie.c libk/*
	eb cc=$(CC)\
		workdir="${WORKDIR}" \
    	flag="${CFLAGS} -c -Ilibk/" \
		ld="ld" ldflag="${LDFLAGS} -Ttext $(VA_USER) -e Main" ldfirst="zombie.c.o" \
		as="$(AS)" asflag="${ASFLAGS}" out=zombie.elf server/zombie.c

# asserts
#asserts.bin: kernel.elf

OS_FILES = ${WORKDIR}/boot.bin ${WORKDIR}/loader.bin ${WORKDIR}/kernel.elf

# image
floppy.img: Makefile workdir $(OS_FILES)
	cat $(OS_FILES) > os.bin
	wc -c os.bin
	dd bs=512 count=2880 if=/dev/zero of=floppy.img > /dev/null 2>&1
	dd if=os.bin of=floppy.img conv=notrunc > /dev/null 2>&1
