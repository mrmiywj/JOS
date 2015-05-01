// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

<<<<<<< HEAD
    if ((err & FEC_WR) == 0)
    {
        panic("pgfault: page fault was not caused by write; %x.\n", utf->utf_fault_va);
    }

    if ((uvpt[PGNUM(addr)] & PTE_COW) == 0)
    {
        panic("pgfault: page fault on page which is not COW %x.\n", utf->utf_fault_va);
    }
=======
	if (!(err & FEC_WR)
		|| !(uvpt[PGNUM(addr)] & PTE_COW)) {
		cprintf("%p\n", addr);
		panic("pgfault(): not %s!", (err & FEC_WR) ? "COW" : "write");
	}

>>>>>>> 5827db0eea0f7099dbea0b0c3971a14244855197
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
    envid_t envid = sys_getenvid();

    //allocate temp page
    if (sys_page_alloc(envid, PFTEMP, PTE_U | PTE_P | PTE_W) < 0)
    {
        panic("pgfault: can't allocate temp page.\n");
    }

    memmove(PFTEMP, (void *)ROUNDDOWN(addr, PGSIZE), PGSIZE);

<<<<<<< HEAD
    if(sys_page_map(envid, PFTEMP, envid, (void *)ROUNDDOWN(addr, PGSIZE), PTE_U | PTE_P | PTE_W) < 0)
    {
        panic("pgfault: can't map temp page to old page.\n");
    }

    if(sys_page_unmap(envid, PFTEMP) < 0)
    {
        panic("pgfault: couldn't unmap page.\n");
    }
=======
	if ((r = sys_page_alloc(0, (void *)PFTEMP, PTE_U | PTE_W)) < 0) {
		panic("pgfault(): sys_page_alloc() failed: %e", r);
	}

	addr = (void *)(((int)addr) & ~0xFFF);
	memcpy((void *)PFTEMP, addr, PGSIZE);

	if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_U | PTE_W)) < 0) {
		panic("pgfault(): sys_page_map() failed: %e", r);
	}

	if ((r = sys_page_unmap(0, PFTEMP)) < 0) {
		panic("pgfault(): sys_page_unmap() failed: %e", r);
	}

>>>>>>> 5827db0eea0f7099dbea0b0c3971a14244855197
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
<<<<<<< HEAD

    envid_t myenvid = sys_getenvid();
    pte_t pte = uvpt[pn];
    int perm;


    if (pte & PTE_SHARE){
    	if ((r = sys_page_map(myenvid,
    						 (void *)(pn * PGSIZE),
    						 envid,
    						 (void *) (pn * PGSIZE),
    						 pte& PTE_SYSCALL)) < 0){
    		return r;
    	}
    }
    // map to envid VA
    else{
        perm = PTE_U | PTE_P;
        if(pte & PTE_W || pte & PTE_COW)
        {
            perm |= PTE_COW;
        }

    	if ((r = sys_page_map(myenvid,

                        (void *)(pn * PGSIZE),
                        envid,
                        (void *) (pn * PGSIZE),
                        perm))
                < 0)
    {
        return r;
    }

    // if COW remap to self
    if(perm & PTE_COW)
    {
        if((r = sys_page_map(myenvid,
                            (void *)(pn * PGSIZE),
                            myenvid,
                            (void *) (pn * PGSIZE),
                            perm))
                    < 0)
        {
            return r;
        }
    }
    }
=======
	//panic("duppage not implemented");

	int perm;
	void * va = (void *)(pn * PGSIZE);

	if (uvpt[pn] & PTE_SHARE) {
		perm = uvpt[pn] & PTE_SYSCALL;
		if ((r = sys_page_map(0, va, envid, va, perm)) < 0) {
			panic("duppage(): sys_page_map() copy shared failed: %e", r);
		}
		return 0;
	}

	if (uvpt[pn] & (PTE_W | PTE_COW)) {
		perm = PTE_COW | PTE_U | PTE_P;
	}
	else {
		perm = PTE_U | PTE_P;
	}

	if ((r = sys_page_map(0, va, envid, va, perm)) < 0) {
		panic("duppage(): sys_page_map() failed: %e\nva: %p\tenvid: %d\tperm: %08x\n", r, va, envid, perm);
	}

	if ((perm & PTE_COW) 
		&& (r = sys_page_map(0, va, 0, va, perm)) < 0) {
		panic("duppage(): sys_page_map() for self failed: %e\nva: %p\tperm: %08x\n", r, va, perm);
	}

>>>>>>> 5827db0eea0f7099dbea0b0c3971a14244855197
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
<<<<<<< HEAD
    extern void _pgfault_upcall(void);
    envid_t myenvid = sys_getenvid();
    envid_t envid;
    uint32_t i, j, pn;

    //set page fault handler
    set_pgfault_handler(pgfault);

    //create a child
    if((envid = sys_exofork()) < 0)
    {
        return -1;
    }

    if(envid == 0)
    {
        thisenv = &envs[ENVX(sys_getenvid())];

        return envid;
    }

    //copy address space to child
    for (i = PDX(UTEXT); i < PDX(UXSTACKTOP); i++)
    {
        if(uvpd[i] & PTE_P)
        {
            for (j = 0; j < NPTENTRIES; j++)
            {
                pn = PGNUM(PGADDR(i, j, 0));
                if(pn == PGNUM(UXSTACKTOP - PGSIZE))
                {
                    break;
                }

                if(uvpt[pn] & PTE_P)
                {
                    duppage(envid, pn);
                }
            }
        }
    }

    if(sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_P | PTE_W) < 0)
    {
        return -1;
    }

    if(sys_page_map(envid, (void *)(UXSTACKTOP - PGSIZE), myenvid, PFTEMP, PTE_U | PTE_P | PTE_W) < 0)
    {
        return -1;
    }

    memmove((void *)(UXSTACKTOP - PGSIZE), PFTEMP, PGSIZE);

    if(sys_page_unmap(myenvid, PFTEMP) < 0)
    {
        return -1;
    }

    if(sys_env_set_pgfault_upcall(envid, _pgfault_upcall) < 0)
    {
        return -1;
    }

    if(sys_env_set_status(envid, ENV_RUNNABLE) < 0)
    {
        return -1;
    }

    return envid;
    //	panic("fork not implemented");
=======
	//panic("fork not implemented");

	envid_t child;
	int r;
	void * va;

	set_pgfault_handler(pgfault);

	if ((child = sys_exofork()) < 0) {
		panic("fork(): sys_exofork() failed: %e", child);
	}
	else if (child == 0) {
		thisenv = getThisenv();
	}
	else {
		
		for (va = (void *)UTEXT; va < (void *)USTACKTOP; va += PGSIZE) {
			
			if (!(uvpd[PDX(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_U)) {
				continue;
			}

			duppage(child, PGNUM(va));
		}

		sys_page_alloc(child, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W);

		extern void _pgfault_upcall();

		if ((r = sys_env_set_pgfault_upcall(child, _pgfault_upcall)) < 0) {
			panic("fork(): sys_env_set_pgfault_upcall() failed: %e", r);
		}

		if ((r = sys_env_set_status(child, ENV_RUNNABLE)) < 0) {
			panic("fork(): sys_env_setstatus() failed: %e", r);
		}

	}

	return child;
>>>>>>> 5827db0eea0f7099dbea0b0c3971a14244855197
}

// Challenge!
int
sfork(void)
{
<<<<<<< HEAD
	// LAB 4: Your code here.
    extern void _pgfault_upcall(void);
    envid_t myenvid = sys_getenvid();
    envid_t envid;
    uint32_t i, j, pn;
    int perm;

    // set page fault handler
    set_pgfault_handler(pgfault);

    // create a child
    if((envid = sys_exofork()) < 0)
    {
        return -1;
    }

    if(envid == 0)
    {
        thisenv = &envs[ENVX(sys_getenvid())];
        return envid;
    }

    // copy address space to child
    for (i = PDX(UTEXT); i < PDX(UXSTACKTOP); i++)
    {
        if(uvpd[i] & PTE_P)
        {
            for (j = 0; j < NPTENTRIES; j++)
            {
                pn = PGNUM(PGADDR(i, j, 0));
                if(pn == PGNUM(UXSTACKTOP - PGSIZE))
                {
                    break;
                }

                if(pn == PGNUM(USTACKTOP - PGSIZE))
                {
                     duppage(envid, pn); // cow for stack page
                     continue;
                }

                // map same page to child env with same perms
                if (uvpt[pn] & PTE_P)
                {

                    perm = uvpt[pn] & ~(uvpt[pn] & ~(PTE_P |PTE_U | PTE_W | PTE_AVAIL));
                    if (sys_page_map(myenvid, (void *)(PGADDR(i, j, 0)),
                                     envid,   (void *)(PGADDR(i, j, 0)), perm) < 0)
                    {
                        return -1;
                    }

                }
            }
        }
    }

    // allocate new exception stack for child
    if(sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_P | PTE_W) < 0)
    {
        return -1;
    }

    // map child uxstack to temp page
    if(sys_page_map(envid, (void *)(UXSTACKTOP - PGSIZE), myenvid, PFTEMP, PTE_U | PTE_P | PTE_W) < 0)
    {
        return -1;
    }

    // copy own uxstack to temp page
    memmove((void *)(UXSTACKTOP - PGSIZE), PFTEMP, PGSIZE);

    if(sys_page_unmap(myenvid, PFTEMP) < 0)
    {
        return -1;
    }

    // set page fault handler in child
    if(sys_env_set_pgfault_upcall(envid, _pgfault_upcall) < 0)
    {
        return -1;
    }

    // mark child env as RUNNABLE
    if(sys_env_set_status(envid, ENV_RUNNABLE) < 0)
    {
        return -1;
    }

    return envid;
=======
	//panic("sfork not implemented");
	envid_t child;
	int r;
	void * va;

	set_pgfault_handler(pgfault);

	if ((child = sys_exofork()) < 0) {
		panic("sfork(): sys_exofork() failed: %e", child);
	}
	else if (child > 0) {

		cprintf("child: %d\n", child);

		va = (void *)(USTACKTOP - PGSIZE);
		r = 0;

		while ((uvpd[PDX(va)] & PTE_P)
				&& (uvpt[PGNUM(va)] & PTE_P)) {
			duppage(child, PGNUM(va));
			va -= PGSIZE;
		}

		for (; va >= (void *)UTEXT; va -= PGSIZE) {

			if (!(uvpd[PDX(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_P)
				|| !(uvpt[PGNUM(va)] & PTE_U)) {
				continue;
			}

			if (sys_page_map(0, va, child, va, PTE_U | PTE_P | (uvpt[PGNUM(va)] & PTE_W)) < 0) {
				panic("sfork(): sys_page_map() error!");
			}
		}

		sys_page_alloc(child, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W);

		extern void _pgfault_upcall();

		if ((r = sys_env_set_pgfault_upcall(child, _pgfault_upcall)) < 0) {
			panic("sfork(): sys_env_set_pgfault_upcall() failed: %e", r);
		}

		if ((r = sys_env_set_status(child, ENV_RUNNABLE)) < 0) {
			panic("sfork(): sys_env_setstatus() failed: %e", r);
		}

	}

	return child;
>>>>>>> 5827db0eea0f7099dbea0b0c3971a14244855197
}
