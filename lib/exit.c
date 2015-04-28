
#include <inc/lib.h>

void
exit(void)
{
	//at start of lab5
	//close_all();
	sys_env_destroy(0);
}

