#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

/**

 * shim to create missing function call in ingenic library

 */
void __pthread_register_cancel(void *buf){}
void __pthread_unregister_cancel(void *buf){}
void __assert (const char *msg, const char *file, int line){}
int __fgetc_unlocked(FILE *__stream)

{
  int iVar1;
  iVar1 = fgetc(__stream);
  return iVar1;
}

void * mmap(void *__addr,size_t __len,int __prot,int __flags,int __fd,off_t __offset)
{

  asm volatile(	"addiu  $sp,$sp,-48\n\t"	// reserve some stack space for channing the call to mmap64
		".extern mmap64\n\t"		// let the linker know we'll be linking to mmap64
		".set    noreorder\n\t"		// GOT https://www.linux-mips.org/wiki/PIC_code
 		".cpload $25\n\t"		// GOT https://www.linux-mips.org/wiki/PIC_code
 		".set    reorder\n\t"		// GOT https://www.linux-mips.org/wiki/PIC_code
		"lw	$v0,68($sp)\n\t"	// load the mmap len to $v0
		"sw	$ra,44($sp)\n\t"	// store the return addr to local variable
		"sw	$gp,32($sp)\n\t"	// store the gp to local variable
		"sw	$v0,24($sp)\n\t"	// store the len to the fixedup mmap paramater location?
		"sw	$zero,28($sp)\n\t"	// store the extended len(?) parameter
		"lw	$v0,64($sp)\n\t"	// seems unnecessary, overwrites the calling paramter length
		"sw	$v0,16($sp)\n\t"	// store the len to ?
		"jal	mmap64\n\t"		// call mmap throught the GOT
		"lw	$ra,44($sp)\n\t"	// recover the return address
		"addiu	$sp,$sp,48\n\t"		// free our stack space
		"" : );
}
