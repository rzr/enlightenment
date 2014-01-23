#ifdef E_TYPEDEFS

//typedef struct _E_Screen E_Screen;

typedef enum _E_Backlight_Mode
{
   E_BACKLIGHT_MODE_NORMAL = 0,
   E_BACKLIGHT_MODE_OFF = 1,
   E_BACKLIGHT_MODE_MIN = 2,
   E_BACKLIGHT_MODE_DIM = 3,
   E_BACKLIGHT_MODE_MAX = 4
     // for the future. right now not working as we'd need an optical
     // sensor support framework
//   E_BACKLIGHT_MODE_AUTO = 5
} E_Backlight_Mode;

#else
# ifndef E_BACKLIGHT_H
#  define E_BACKLIGHT_H

# endif
#endif
