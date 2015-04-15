#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#if defined(HAVE_EEZE)
#include <Eeze.h>

/* local subsystem functions */
static int
_bl_write_file(const char *file, int val)
{
   char buf[256];
   int fd = open(file, O_WRONLY);
   if (fd < 0)
     {
        perror("open");
        return -1;
     }
   snprintf(buf, sizeof(buf), "%d", val);
   if (write(fd, buf, strlen(buf)) != (int)strlen(buf))
     {
        perror("write");
        close(fd);
        return -1;
     }
   close(fd);
   return 0;
}

static int
_bl_set(const char *dev, int level)
{
   const char *f, *str;
   int maxlevel = 0, curlevel = -1;
   Eina_List *devs, *l;
   Eina_Bool devok = EINA_FALSE;
   char buf[4096] = "";

   eeze_init();
   devs = eeze_udev_find_by_filter("backlight", NULL, NULL);
   if (!devs)
     {
        devs = eeze_udev_find_by_filter("leds", NULL, NULL);
        if (!devs) return -1;
     }
   if (devs)
     {
        EINA_LIST_FOREACH(devs, l, f)
          {
             if (!strcmp(f, dev))
               {
                  dev = f;
                  devok = EINA_TRUE;
                  break;
               }
          }
     }

   if (!devok) return -1;

   str = eeze_udev_syspath_get_sysattr(dev, "max_brightness");
   if (str)
     {
        maxlevel = atoi(str);
        eina_stringshare_del(str);
        str = eeze_udev_syspath_get_sysattr(dev, "brightness");
        if (str)
          {
             curlevel = atoi(str);
             eina_stringshare_del(str);
          }
     }

   if (maxlevel <= 0) maxlevel = 255;
   if (curlevel >= 0)
     {
        curlevel = ((maxlevel * level) + 500) / 1000;
        if (curlevel > maxlevel) curlevel = maxlevel;
        else if (curlevel < 0)
          curlevel = 0;
        snprintf(buf, sizeof(buf), "%s/brightness", f);
        return _bl_write_file(buf, curlevel);
     }

   EINA_LIST_FREE(devs, f)
     eina_stringshare_del(f);
   return -1;
}
#elif defined(__FreeBSD_kernel__)  // !HAVE_EEZE
#include <sys/sysctl.h>
#include <errno.h>

static const char *bl_acpi_sysctl = "hw.acpi.video.lcd0.brightness";

static int
_bl_set(const char *dev, int level)
{
   int rc;

   level = ((100 * level) + 500) / 1000;
   if (level > 100) level = 100;
   else if (level < 0) level = 0;

   // Be slightly careful if making this more permissive. We don't want to
   // allow non-root users to set arbitrary integer sysctls between 0-100.
   if (strcmp(bl_acpi_sysctl, dev) != 0)
     {
        printf("bad device: %s\n", dev);
        return -1;
     }

   rc = sysctlbyname(bl_acpi_sysctl, NULL, NULL, &level, sizeof(level));
   if (rc < 0)
     {
        perror("sysctlbyname");
        return -1;
     }
   return 0;
}
#endif  // __FreeBSD_kernel__

#if defined(HAVE_EEZE) || defined(__FreeBSD_kernel__)
int
main(int argc, char **argv)
{
   const char *dev = NULL;
   int i, level;

   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-h")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "--help")))
          {
             printf("This is an internal tool for Enlightenment.\n"
                    "do not use it.\n");
             exit(0);
          }
     }
   if (argc == 3)
     {
        level = atoi(argv[1]);
        dev = argv[2];
     }
   else
     exit(1);

   if (!dev) return -1;

   if (setuid(0) != 0)
     {
        printf("ERROR: UNABLE TO ASSUME ROOT PRIVILEGES\n");
        exit(5);
     }
   if (setgid(0) != 0)
     {
        printf("ERROR: UNABLE TO ASSUME ROOT GROUP PRIVILEGES\n");
        exit(7);
     }

   return _bl_set(dev, level);
}
#else  // !HAVE_EEZE && !__FreeBSD_kernel__
int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   return -1;
}
#endif
