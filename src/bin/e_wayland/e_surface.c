#include "e.h"

/* local function prototypes */
static void _e_surface_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource);
static void _e_surface_cb_attach(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *buffer_resource, int x, int y);
static void _e_surface_cb_damage(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int x, int y, int w, int h);
static void _e_surface_cb_commit(struct wl_client *client EINA_UNUSED, struct wl_resource *resource);
static void _e_surface_cb_frame(struct wl_client *client, struct wl_resource *resource, unsigned int id);
static void _e_surface_cb_opaque_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource);
static void _e_surface_cb_input_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource);
static void _e_surface_cb_buffer_destroy(struct wl_listener *listener, void *data EINA_UNUSED);

static void _e_surface_frame_cb_destroy(struct wl_resource *resource);

/* local wayland interfaces */
static const struct wl_surface_interface _e_surface_interface = 
{
   _e_surface_cb_destroy,
   _e_surface_cb_attach,
   _e_surface_cb_damage,
   _e_surface_cb_frame,
   _e_surface_cb_opaque_set,
   _e_surface_cb_input_set,
   _e_surface_cb_commit,
   NULL // cb_buffer_transform_set
};

EAPI E_Surface *
e_surface_new(unsigned int id)
{
   E_Surface *es;

   /* try to allocate space for a new surface */
   if (!(es = E_NEW(E_Surface, 1))) return NULL;

   /* initialize the destroy signal */
   wl_signal_init(&es->wl.resource.destroy_signal);

   /* initialize the link */
   wl_list_init(&es->wl.link);

   pixman_region32_init(&es->damage);
   pixman_region32_init(&es->opaque);
   pixman_region32_init(&es->clip);
   pixman_region32_init_rect(&es->input, INT32_MIN, INT32_MIN, 
                             UINT32_MAX, UINT32_MAX);

   /* TODO: finish me */

   es->pending.buffer_destroy.notify = _e_surface_cb_buffer_destroy;

   pixman_region32_init(&es->pending.damage);
   pixman_region32_init(&es->pending.opaque);
   pixman_region32_init_rect(&es->pending.input, INT32_MIN, INT32_MIN, 
                             UINT32_MAX, UINT32_MAX);

   /* setup the surface object */
   es->wl.resource.object.id = id;
   es->wl.resource.object.interface = &wl_surface_interface;
   es->wl.resource.object.implementation = 
     (void (**)(void))&_e_surface_interface;
   es->wl.resource.data = es;

   return es;
}

EAPI void 
e_surface_attach(E_Surface *es, struct wl_buffer *buffer)
{
   /* check for valid surface */
   if (!es) return;

   /* reference this buffer */
   e_buffer_reference(&es->buffer.reference, buffer);

   if (!buffer)
     {
        /* if we have no buffer to attach (buffer == NULL), 
         * then we assume that we want to unmap this surface (hide it) */

        /* FIXME: This may need to call the surface->unmap function if a 
         * shell wants to do something "different" when it unmaps */
        if (es->mapped) e_surface_unmap(es);
     }

   /* call renderer attach */
   if (_e_comp->attach) _e_comp->attach(es, buffer);
}

EAPI void 
e_surface_unmap(E_Surface *es)
{
   /* TODO */
}

EAPI void 
e_surface_damage(E_Surface *es)
{
   /* check for valid surface */
   if (!es) return;

   /* add this damage rectangle */
   pixman_region32_union_rect(&es->damage, &es->damage, 
                              0, 0, es->geometry.w, es->geometry.h);

   /* TODO: schedule repaint */
}

EAPI void 
e_surface_destroy(E_Surface *es)
{
   E_Surface_Frame *cb;

   /* check for valid surface */
   if (!es) return;

   /* emit the destroy signal */
   wl_signal_emit(&es->wl.resource.destroy_signal, 
                  &es->wl.resource);

   /* if this surface is mapped, unmap it */
   if (es->mapped) e_surface_unmap(es);

   /* remove any pending frame callbacks */
   EINA_LIST_FREE(es->pending.frames, cb)
     wl_resource_destroy(&cb->resource);

   pixman_region32_fini(&es->pending.damage);
   pixman_region32_fini(&es->pending.opaque);
   pixman_region32_fini(&es->pending.input);

   /* destroy pending buffer */
   if (es->pending.buffer)
     wl_list_remove(&es->pending.buffer_destroy.link);

   /* remove any buffer references */
   e_buffer_reference(&es->buffer.reference, NULL);

   /* free regions */
   pixman_region32_fini(&es->damage);
   pixman_region32_fini(&es->opaque);
   pixman_region32_fini(&es->input);

   /* remove any active frame callbacks */
   EINA_LIST_FREE(es->frames, cb)
     wl_resource_destroy(&cb->resource);

   /* free the surface structure */
   E_FREE(es);
}

EAPI void 
e_surface_damage_calculate(E_Surface *es, pixman_region32_t *opaque)
{
   /* check for valid surface */
   if (!es) return;

   /* check for referenced buffer */
   if (&es->buffer.reference.buffer)
     {
        /* if this is an shm buffer, flush any pending damage */
        if (wl_buffer_is_shm(es->buffer.reference.buffer))
          e_compositor_damage_flush(_e_comp, es);
     }

   /* TODO: handle transforms */

   pixman_region32_translate(&es->damage, 
                             es->geometry.x - es->plane->x, 
                             es->geometry.y - es->plane->y);

   pixman_region32_subtract(&es->damage, &es->damage, opaque);
   pixman_region32_union(&es->plane->damage, &es->plane->damage, &es->damage);

   pixman_region32_fini(&es->damage);
   pixman_region32_init(&es->damage);

   pixman_region32_copy(&es->clip, opaque);
}

/* local functions */
static void 
_e_surface_cb_destroy(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   wl_resource_destroy(resource);
}

static void 
_e_surface_cb_attach(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *buffer_resource, int x, int y)
{
   E_Surface *es;
   struct wl_buffer *buffer = NULL;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   /* if we have a buffer resource, get a wl_buffer from it */
   if (buffer_resource) buffer = buffer_resource->data;

   /* if we have a previous pending buffer, remove it
    * 
    * NB: This means that attach was called more than once without calling 
    * a commit in between, so we disgard any old buffers and just deal with 
    * the most current request */
   if (es->pending.buffer) wl_list_remove(&es->pending.buffer_destroy.link);

   /* set surface pending properties */
   es->pending.x = x;
   es->pending.y = y;
   es->pending.buffer = buffer;
   es->pending.new_attach = EINA_TRUE;

   /* if we have a valid pending buffer, setup a destroy listener */
   if (buffer) 
     wl_signal_add(&buffer->resource.destroy_signal, 
                   &es->pending.buffer_destroy);
}

static void 
_e_surface_cb_damage(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, int x, int y, int w, int h)
{
   E_Surface *es;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   /* add this damage rectangle */
   pixman_region32_union_rect(&es->pending.damage, &es->pending.damage, 
                              x, y, w, h);
}

static void 
_e_surface_cb_commit(struct wl_client *client EINA_UNUSED, struct wl_resource *resource)
{
   E_Surface *es;
   E_Surface_Frame *cb;
   Evas_Coord bw = 0, bh = 0;
   pixman_region32_t opaque;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   /* if we have a pending buffer, attach it */
   if ((es->pending.buffer) || (es->pending.new_attach))
     e_surface_attach(es, es->pending.buffer);

   /* if we have a referenced buffer, get it's size */
   if (es->buffer.reference.buffer)
     {
        bw = es->buffer.reference.buffer->width;
        bh = es->buffer.reference.buffer->height;
     }

   /* if we attached a new buffer, call the surface configure function */
   if ((es->pending.new_attach) && (es->configure))
     es->configure(es, es->pending.x, es->pending.y, bw, bh);

   /* remove the destroy listener for the pending buffer */
   if (es->pending.buffer) wl_list_remove(&es->pending.buffer_destroy.link);

   /* clear surface pending properties */
   es->pending.buffer = NULL;
   es->pending.x = 0;
   es->pending.y = 0;
   es->pending.new_attach = EINA_FALSE;

   /* combine any pending damage */
   pixman_region32_union(&es->damage, &es->damage, &es->pending.damage);
   pixman_region32_intersect_rect(&es->damage, &es->damage, 
                                  0, 0, es->geometry.w, es->geometry.h);
   /* TODO: empty region */

   /* free any pending damage */
   pixman_region32_fini(&es->pending.damage);
   pixman_region32_init(&es->pending.damage);

   /* combine any pending opaque */
   pixman_region32_init_rect(&opaque, 0, 0, es->geometry.w, es->geometry.h);
   pixman_region32_intersect(&opaque, &opaque, &es->pending.opaque);
   if (!pixman_region32_equal(&opaque, &es->opaque))
     {
        pixman_region32_copy(&es->opaque, &opaque);
        es->geometry.changed = EINA_TRUE;
     }

   /* combine any pending input */
   pixman_region32_fini(&es->input);
   pixman_region32_init_rect(&es->input, 0, 0, es->geometry.w, es->geometry.h);
   pixman_region32_intersect(&es->input, &es->input, &es->pending.input);

   /* add any pending frame callbacks to main list and free pending */
   EINA_LIST_FREE(es->pending.frames, cb)
     es->frames = eina_list_append(es->frames, cb);

   /* schedule repaint */
   /* evas_damage_rectangle_add(es->evas, es->damage->x, es->damage->y,  */
   /*                           es->damage->w, es->damage->h); */
}

static void 
_e_surface_cb_frame(struct wl_client *client, struct wl_resource *resource, unsigned int id)
{
   E_Surface *es;
   E_Surface_Frame *cb;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   /* try to create a new frame callback */
   if (!(cb = E_NEW_RAW(E_Surface_Frame, 1)))
     {
        wl_resource_post_no_memory(resource);
        return;
     }

   cb->surface = es;

   /* setup the callback object */
   cb->resource.object.interface = &wl_callback_interface;
   cb->resource.object.id = id;
   cb->resource.destroy = _e_surface_frame_cb_destroy;
   cb->resource.client = client;
   cb->resource.data = cb;

   /* add this callback to the client */
   wl_client_add_resource(client, &cb->resource);

   /* append the callback to pending frames */
   es->pending.frames = eina_list_prepend(es->pending.frames, cb);
}

static void 
_e_surface_cb_opaque_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   E_Surface *es;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   if (region_resource)
     {
        E_Region *reg;

        /* try to cast this resource to our region */
        reg = region_resource->data;
        pixman_region32_copy(&es->pending.opaque, &reg->region);
     }
   else
     {
        pixman_region32_fini(&es->pending.opaque);
        pixman_region32_init(&es->pending.opaque);
     }
}

static void 
_e_surface_cb_input_set(struct wl_client *client EINA_UNUSED, struct wl_resource *resource, struct wl_resource *region_resource)
{
   E_Surface *es;

   /* try to cast the resource to our surface */
   if (!(es = resource->data)) return;

   if (region_resource)
     {
        E_Region *reg;

        /* try to cast this resource to our region */
        reg = region_resource->data;
        pixman_region32_copy(&es->pending.input, &reg->region);
     }
   else
     {
        pixman_region32_fini(&es->pending.input);
        pixman_region32_init_rect(&es->pending.input, INT32_MIN, INT32_MIN, 
                                  UINT32_MAX, UINT32_MAX);
     }
}

static void 
_e_surface_cb_buffer_destroy(struct wl_listener *listener, void *data EINA_UNUSED)
{
   E_Surface *es;

   /* try to get the surface structure from the listener */
   if (!(es = container_of(listener, E_Surface, pending.buffer_destroy)))
     return;

   /* clear the pending buffer */
   es->pending.buffer = NULL;
}

static void 
_e_surface_frame_cb_destroy(struct wl_resource *resource)
{
   E_Surface *es;
   E_Surface_Frame *cb;

   /* try to cast the resource to our callback */
   if (!(cb = resource->data)) return;

   es = cb->surface;

   /* remove this callback from the pending frames callback list */
   es->pending.frames = eina_list_remove(es->pending.frames, cb);

   /* remove this callback from the frames callback list */
   es->frames = eina_list_remove(es->frames, cb);

   /* free the callback structure */
   E_FREE(cb);
}