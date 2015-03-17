// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/attributed.h>

#include <kern/pmap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display the trace of the stack", mon_backtrace },
#ifdef MAPPINGDEBUG
	{ "showmapping", "Display the mapping for the physical memory address", mon_showmapping },
#endif
	{ "cutytest", "cuty-lewiwi needs to test some functions", mon_cutytest},
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	int *ebp = (int *)read_ebp();
	struct Eipdebuginfo info;
	cprintf("Stack backtrace:\n");
	do {
		debuginfo_eip(ebp[1], &info);
		cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", 
			(int *)ebp, ebp[1], ebp[2], ebp[3], ebp[4], ebp[5], ebp[6]);
		cprintf("         %s:%d: %.*s+%d\n", 
			info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, ebp[1] - info.eip_fn_addr);
	} while ((ebp = (int *)(*ebp)) != NULL);

	return 0;
}


// use to test some functions
int
mon_cutytest(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("cutytest:\n");
	//cprintf("%d %d %d\n", COLOR_BLUE, COLOR_GREEN, COLOR_RED);
	//cprintf("%m%s\n%m%s\n%m%s\n", COLOR_BLUE, "blue", COLOR_GREEN, "green", COLOR_RED, "red");
	//int x = 1, y = 3, z = 4;
	//cprintf("x %d, y %x, z %d\n", x, y, z);
	cprintf("%s: %d\n%s: %d\n%s: %d\n", 
		"090", my_atoi("090"), "0x10", my_atoi("0x10"), "10", my_atoi("10"));
    cprintf("\n");
	return 0;
}

#ifdef MAPPINGDEBUG
// showmapping for lab 2 challenge
int
mon_showmapping(int argc, char **argv, struct Trapframe *tf)
{
	int i, end;

	if (argc != 3){
		cprintf("usage: <%s> -start -end\n", argv[0]);
		return 0;
	}

	for (i = my_atoi(argv[1]), end = my_atoi(argv[2]); i <= end; i++){
		
	}

	return 0;
}

int 
my_atoi(char * input)
{
	int result = 0;
	int mode = 0;
	// 0: origin
	// 1: first 0
	// 2: 16 base or 8 base
	// 3: 10 base
	int base = 10;
	int delta = 0;
	char * origin = input;
	while (*input){
		switch (*input){
			case '0':
				if (!mode){
					mode = 1;
				}
				break;
			case 'x':
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				if (mode == 1){
					mode = 2;
					base = 8;
				}
				// fall through
			case '8':
			case '9':
				delta = (*input) - '0';
				if (!mode){
					mode = 3;
					base = 10;
				}
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				delta = (*input) - 'a' + 10;
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				delta = (*input) - 'A' + 10;
				if (mode == 1){
					mode = 2;
					base = 16;
				}
				break;
			default:
				cprintf("my_atoi: cant recognized input: %s\n", origin);
				return -1;
		}
		if (delta >= base){
			cprintf("my_atoi: illegal input string!\n");
			return -1;
		}
		result = result * base + delta;
		delta = 0;
		input ++;
	}
	return result;
}

#endif

/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
