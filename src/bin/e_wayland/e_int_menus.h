#ifdef E_TYPEDEFS

#else
# ifndef E_INT_MENUS_H
#  define E_INT_MENUS_H

typedef enum
{
   E_CLIENTLIST_GROUP_NONE,
   E_CLIENTLIST_GROUP_DESK,
   E_CLIENTLIST_GROUP_CLASS
} E_Clientlist_Group_Type;

typedef enum
{
   E_CLIENTLIST_GROUP_SEP_NONE,
   E_CLIENTLIST_GROUP_SEP_BAR,
   E_CLIENTLIST_GROUP_SEP_MENU
} E_Clientlist_Group_Sep_Type;

typedef enum
{
   E_CLIENTLIST_SORT_NONE,
   E_CLIENTLIST_SORT_ALPHA,
   E_CLIENTLIST_SORT_ZORDER,
   E_CLIENTLIST_SORT_MOST_RECENT
} E_Clientlist_Group_Sort_Type;

typedef enum
{
   E_CLIENTLIST_GROUPICONS_OWNER,
   E_CLIENTLIST_GROUPICONS_CURRENT,
   E_CLIENTLIST_GROUPICONS_SEP,
} E_Clientlist_Groupicons_Type;

#  define E_CLIENTLIST_MAX_CAPTION_LEN 256

# endif
#endif
