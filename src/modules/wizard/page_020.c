/* Profile chooser */
#include "e_wizard.h"

static const char *profile = NULL;
static Evas_Object *textblock = NULL;

static void
_profile_change(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED)
{
   char buf[PATH_MAX], buf2[PATH_MAX];
   Efreet_Desktop *desk = NULL;

   e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", profile);
   snprintf(buf, sizeof(buf), "%s/profile.desktop", buf2);
   desk = efreet_desktop_new(buf);
   if (desk)
     {
        e_widget_textblock_markup_set(textblock, desk->comment);
        efreet_desktop_free(desk);
     }
   else
     e_widget_textblock_markup_set(textblock, _("Unknown"));

   // enable next once you choose a profile
   e_wizard_button_next_enable_set(1);
}
/*
E_API int
wizard_page_init(E_Wizard_Page *pg EINA_UNUSED, Eina_Bool *need_xdg_desktops EINA_UNUSED, Eina_Bool *need_xdg_icons EINA_UNUSED)
{
   return 1;
}

E_API int
wizard_page_shutdown(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
E_API int
wizard_page_show(E_Wizard_Page *pg)
{
   Evas_Object *o, *of, *ob;
   Eina_List *l, *profiles;
   int i, sel = -1;
   Evas_Object *ilist;

   o = e_widget_list_add(pg->evas, 1, 0);
   e_wizard_title_set(_("Profile"));
   of = e_widget_framelist_add(pg->evas, _("Select one"), 0);

   ob = e_widget_ilist_add(pg->evas, 32 * e_scale, 32 * e_scale, &profile);
   e_widget_size_min_set(ob, 140 * e_scale, 70 * e_scale);
   ilist = ob;
   e_widget_on_change_hook_set(ob, _profile_change, NULL);

   e_widget_ilist_freeze(ob);

   profiles = e_config_profile_list();
   for (i = 0, l = profiles; l; l = l->next)
     {
        Efreet_Desktop *desk = NULL;
        char buf[PATH_MAX], buf2[PATH_MAX], *prof;
        const char *label;
        Evas_Object *ic;

        prof = l->data;
        if (e_config_profile_get())
          {
             if (!strcmp(prof, e_config_profile_get()))
               {
                  free(prof);
                  continue;
               }
          }
        e_prefix_data_snprintf(buf2, sizeof(buf2), "data/config/%s", prof);
        // if it's not a system profile - don't offer it
        if (!ecore_file_is_dir(buf2))
          {
             free(prof);
             continue;
          }
        if (!strcmp(prof, "standard")) sel = i;
        snprintf(buf, sizeof(buf), "%s/profile.desktop", buf2);
        desk = efreet_desktop_new(buf);
        label = prof;
        if ((desk) && (desk->name)) label = desk->name;
        snprintf(buf, sizeof(buf), "%s/icon.edj", buf2);
        if ((desk) && (desk->icon))
          {
             if (eina_str_has_extension(desk->icon, "png"))
               snprintf(buf, sizeof(buf), "%s/%s", buf2, desk->icon);
             else
               snprintf(buf, sizeof(buf), "%s/%s.png", buf2, desk->icon);
          }
        else
          e_prefix_data_concat_static(buf, "data/images/enlightenment.png");
        ic = e_util_icon_add(buf, pg->evas);
        e_widget_ilist_append(ob, ic, label, NULL, NULL, prof);
        free(prof);
        if (desk) efreet_desktop_free(desk);

        /* We incremet here, because we don't want to increment it unless we
         * actually found an item. */
        i++;
     }
   if (profiles) eina_list_free(profiles);

   e_widget_ilist_go(ob);
   e_widget_ilist_thaw(ob);

   e_widget_framelist_object_append(of, ob);

   ob = e_widget_textblock_add(pg->evas);
   e_widget_size_min_set(ob, 140 * e_scale, 70 * e_scale);
   e_widget_textblock_markup_set(ob, _("Select a profile"));
   textblock = ob;

   e_widget_framelist_object_append(of, ob);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   if (sel >= 0) e_widget_ilist_selected_set(ilist, sel);

   evas_object_show(ob);
   evas_object_show(of);
   e_wizard_page_show(o);
//   pg->data = o;
   e_wizard_button_next_enable_set(0);
   return 1; /* 1 == show ui, and wait for user, 0 == just continue */
}

E_API int
wizard_page_hide(E_Wizard_Page *pg EINA_UNUSED)
{
//   evas_object_del(pg->data);
// actually apply profile
   if (!profile) profile = "standard";
   e_config_profile_set(profile);
   e_config_profile_del(e_config_profile_get());
   e_config_load();
   e_config_save();
   return 1;
}
/*
E_API int
wizard_page_apply(E_Wizard_Page *pg EINA_UNUSED)
{
   return 1;
}
*/
