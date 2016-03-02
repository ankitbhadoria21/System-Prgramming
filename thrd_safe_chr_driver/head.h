#include<linux/ioctl.h>
#define MY_READ _IOR('r', 0, char *)
#define MY_WRITE _IOW('r', 1, char *)
#define ASP_CHGACCDIR _IOWR('r', 2, char*)
