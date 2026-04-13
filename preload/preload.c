#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>

int system(const char *cmd)
{
    static int (*real_system)(const char *) = NULL;

    if (!real_system)
        real_system = dlsym(RTLD_NEXT, "system");

    if (cmd && strstr(cmd, "gnome-terminal"))
    {
        return 1; /* oops, gnome-terminal doesn't exist 😈 */
    }

    return real_system(cmd);
}
