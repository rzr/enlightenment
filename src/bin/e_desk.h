#ifdef E_TYPEDEFS

typedef struct _E_Desk E_Desk;
typedef struct _E_Event_Desk_Show E_Event_Desk_Show;
typedef struct _E_Event_Desk_Before_Show E_Event_Desk_Before_Show;
typedef struct _E_Event_Desk_After_Show E_Event_Desk_After_Show;
typedef struct _E_Event_Desk_Name_Change E_Event_Desk_Name_Change;
typedef struct _E_Event_Desk_Window_Profile_Change E_Event_Desk_Window_Profile_Change;

typedef void (*E_Desk_Flip_Cb)(void *data, E_Desk *desk, int dx, int dy, Eina_Bool show);

#else
#ifndef E_DESK_H
#define E_DESK_H

#define E_DESK_TYPE 0xE0b01005

typedef enum
{
   E_DESKFLIP_ANIMATION_MODE_OFF,
   E_DESKFLIP_ANIMATION_MODE_PANE,
   E_DESKFLIP_ANIMATION_MODE_ZOOM
} E_Deskflip_Animation_Mode;

struct _E_Desk
{
   E_Object             e_obj_inherit;

   E_Zone              *zone;
   Eina_Stringshare    *name;
   Eina_Stringshare    *window_profile;
   int                  x, y;
   unsigned char        visible : 1;
   unsigned int         deskshow_toggle : 1;
   Eina_List            *fullscreen_clients;

   Evas_Object         *bg_object;

   unsigned int animate_count;
};

struct _E_Event_Desk_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Before_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_After_Show
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Name_Change
{
   E_Desk   *desk;
};

struct _E_Event_Desk_Window_Profile_Change
{
   E_Desk   *desk;
};

EINTERN int          e_desk_init(void);
EINTERN int          e_desk_shutdown(void);
EAPI E_Desk      *e_desk_new(E_Zone *zone, int x, int y);
EAPI void         e_desk_name_set(E_Desk *desk, const char *name);
EAPI void         e_desk_name_add(int manager, int zone, int desk_x, int desk_y, const char *name);
EAPI void         e_desk_name_del(int manager, int zone, int desk_x, int desk_y);
EAPI void         e_desk_name_update(void);
EAPI void         e_desk_show(E_Desk *desk);
EAPI void         e_desk_deskshow(E_Zone *zone);
EAPI E_Client    *e_desk_last_focused_focus(E_Desk *desk);
EAPI E_Client    *e_desk_client_top_visible_get(const E_Desk *desk);
EAPI E_Desk      *e_desk_current_get(E_Zone *zone);
EAPI E_Desk      *e_desk_at_xy_get(E_Zone *zone, int x, int y);
EAPI E_Desk      *e_desk_at_pos_get(E_Zone *zone, int pos);
EAPI void         e_desk_xy_get(E_Desk *desk, int *x, int *y);
EAPI void         e_desk_next(E_Zone *zone);
EAPI void         e_desk_prev(E_Zone *zone);
EAPI void         e_desk_row_add(E_Zone *zone);
EAPI void         e_desk_row_remove(E_Zone *zone);
EAPI void         e_desk_col_add(E_Zone *zone);
EAPI void         e_desk_col_remove(E_Zone *zone);
EAPI void         e_desk_window_profile_set(E_Desk *desk, const char *profile);
EAPI void         e_desk_window_profile_add(int manager, int zone, int desk_x, int desk_y, const char *profile);
EAPI void         e_desk_window_profile_del(int manager, int zone, int desk_x, int desk_y);
EAPI void         e_desk_window_profile_update(void);

EAPI void         e_desk_flip_cb_set(E_Desk_Flip_Cb cb, const void *data);
EAPI void         e_desk_flip_end(E_Desk *desk);

EAPI unsigned int e_desks_count(void);

extern EAPI int E_EVENT_DESK_SHOW;
extern EAPI int E_EVENT_DESK_BEFORE_SHOW;
extern EAPI int E_EVENT_DESK_AFTER_SHOW;
extern EAPI int E_EVENT_DESK_DESKSHOW;
extern EAPI int E_EVENT_DESK_NAME_CHANGE;
extern EAPI int E_EVENT_DESK_WINDOW_PROFILE_CHANGE;

#endif
#endif
