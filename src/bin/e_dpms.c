#include "e.h"

static Ecore_Event_Handler *_e_dpms_handler_config_mode = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_fullscreen = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_unfullscreen = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_remove = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_iconify = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_uniconify = NULL;
static Ecore_Event_Handler *_e_dpms_handler_border_desk_set = NULL;
static Ecore_Event_Handler *_e_dpms_handler_desk_show = NULL;

static unsigned int _e_dpms_timeout_standby = 0;
static unsigned int _e_dpms_timeout_suspend = 0;
static unsigned int _e_dpms_timeout_off = 0;
static int _e_dpms_enabled = EINA_FALSE;

E_API void
e_dpms_update(void)
{
   unsigned int standby = 0, suspend = 0, off = 0;
   int enabled;
   Eina_Bool changed = EINA_FALSE;

   enabled = ((e_config->screensaver_enable) &&
              (!e_config->mode.presentation) &&
              ((!e_util_fullscreen_current_any()) && (!e_config->no_dpms_on_fullscreen))
             );
   if (_e_dpms_enabled != enabled)
     {
        _e_dpms_enabled = enabled;
#ifndef HAVE_WAYLAND_ONLY
        if (e_comp_util_has_x())
          ecore_x_dpms_enabled_set(enabled);
#endif
     }
   if (!enabled) return;

   if (e_config->screensaver_enable)
     {
        off = suspend = standby = e_screensaver_timeout_get(EINA_FALSE);
        standby += 5;
        suspend += 6;
        off += 7;
     }
   if (_e_dpms_timeout_standby != standby)
     {
        _e_dpms_timeout_standby = standby;
        changed = EINA_TRUE;
     }
   if (_e_dpms_timeout_suspend != suspend)
     {
        _e_dpms_timeout_suspend = suspend;
        changed = EINA_TRUE;
     }
   if (_e_dpms_timeout_off != off)
     {
        _e_dpms_timeout_off = off;
        changed = EINA_TRUE;
     }
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        if (changed) ecore_x_dpms_timeouts_set(standby, suspend, off);
     }
#endif
}

E_API void
e_dpms_force_update(void)
{
   unsigned int standby = 0, suspend = 0, off = 0;
   int enabled;

   enabled = ((e_config->screensaver_enable) &&
              (!e_config->mode.presentation));
#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     ecore_x_dpms_enabled_set(enabled);
#endif
   if (!enabled) return;

   if (e_config->screensaver_enable)
     {
        off = suspend = standby = e_screensaver_timeout_get(EINA_FALSE);
        standby += 5;
        suspend += 6;
        off += 7;
     }
#ifndef HAVE_WAYLAND_ONLY
   if (!e_comp_util_has_x()) return;
   ecore_x_dpms_timeouts_set(standby + 10, suspend + 10, off + 10);
   ecore_x_dpms_timeouts_set(standby, suspend, off);
#endif
}

static Eina_Bool
_e_dpms_handler_config_mode_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   e_dpms_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dpms_handler_border_fullscreen_check_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   if (e_config->no_dpms_on_fullscreen) e_dpms_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dpms_handler_border_desk_set_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   if (e_config->no_dpms_on_fullscreen) e_dpms_update();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_e_dpms_handler_desk_show_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   if (e_config->no_dpms_on_fullscreen) e_dpms_update();
   return ECORE_CALLBACK_PASS_ON;
}

EINTERN int
e_dpms_init(void)
{
   _e_dpms_handler_config_mode = ecore_event_handler_add
       (E_EVENT_CONFIG_MODE_CHANGED, _e_dpms_handler_config_mode_cb, NULL);

   _e_dpms_handler_border_fullscreen = ecore_event_handler_add
       (E_EVENT_CLIENT_FULLSCREEN, _e_dpms_handler_border_fullscreen_check_cb, NULL);

   _e_dpms_handler_border_unfullscreen = ecore_event_handler_add
       (E_EVENT_CLIENT_UNFULLSCREEN, _e_dpms_handler_border_fullscreen_check_cb, NULL);

   _e_dpms_handler_border_remove = ecore_event_handler_add
       (E_EVENT_CLIENT_REMOVE, _e_dpms_handler_border_fullscreen_check_cb, NULL);

   _e_dpms_handler_border_iconify = ecore_event_handler_add
       (E_EVENT_CLIENT_ICONIFY, _e_dpms_handler_border_fullscreen_check_cb, NULL);

   _e_dpms_handler_border_uniconify = ecore_event_handler_add
       (E_EVENT_CLIENT_UNICONIFY, _e_dpms_handler_border_fullscreen_check_cb, NULL);

   _e_dpms_handler_border_desk_set = ecore_event_handler_add
       (E_EVENT_CLIENT_DESK_SET, _e_dpms_handler_border_desk_set_cb, NULL);

   _e_dpms_handler_desk_show = ecore_event_handler_add
       (E_EVENT_DESK_SHOW, _e_dpms_handler_desk_show_cb, NULL);

#ifndef HAVE_WAYLAND_ONLY
   if (e_comp_util_has_x())
     {
        _e_dpms_enabled = ecore_x_dpms_enabled_get();
        ecore_x_dpms_timeouts_get
          (&_e_dpms_timeout_standby, &_e_dpms_timeout_suspend, &_e_dpms_timeout_off);
     }
#endif

   e_dpms_force_update();

   return 1;
}

EINTERN int
e_dpms_shutdown(void)
{
   if (_e_dpms_handler_config_mode)
     {
        ecore_event_handler_del(_e_dpms_handler_config_mode);
        _e_dpms_handler_config_mode = NULL;
     }

   if (_e_dpms_handler_border_fullscreen)
     {
        ecore_event_handler_del(_e_dpms_handler_border_fullscreen);
        _e_dpms_handler_border_fullscreen = NULL;
     }

   if (_e_dpms_handler_border_unfullscreen)
     {
        ecore_event_handler_del(_e_dpms_handler_border_unfullscreen);
        _e_dpms_handler_border_unfullscreen = NULL;
     }

   if (_e_dpms_handler_border_remove)
     {
        ecore_event_handler_del(_e_dpms_handler_border_remove);
        _e_dpms_handler_border_remove = NULL;
     }

   if (_e_dpms_handler_border_iconify)
     {
        ecore_event_handler_del(_e_dpms_handler_border_iconify);
        _e_dpms_handler_border_iconify = NULL;
     }

   if (_e_dpms_handler_border_uniconify)
     {
        ecore_event_handler_del(_e_dpms_handler_border_uniconify);
        _e_dpms_handler_border_uniconify = NULL;
     }

   if (_e_dpms_handler_border_desk_set)
     {
        ecore_event_handler_del(_e_dpms_handler_border_desk_set);
        _e_dpms_handler_border_desk_set = NULL;
     }

   if (_e_dpms_handler_desk_show)
     {
        ecore_event_handler_del(_e_dpms_handler_desk_show);
        _e_dpms_handler_desk_show = NULL;
     }

   return 1;
}

