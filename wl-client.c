#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct wl_compositor* comp;
struct wl_surface* srfc;
struct wl_buffer* bfr;
struct wl_shm* shm;
struct xdg_wm_base* sh; // xdg shell
struct xdg_toplevel* top; // main window
uint8_t* pixl;
uint16_t w = 200;
uint16_t h = 100;

// Allocates shared memory.
int32_t
alc_shm(uint64_t sz)
{
    int8_t name[8];
    name[0] = '/';
    name[7] = 0;
    for (uint8_t i = 1; i < 6; i++)
    {
        // Just some random letters.
        name[i] = (rand() & 23) + 97;
    }

    // File descriptor.
    int32_t fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL,
            S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
    shm_unlink(name);
    ftruncate(fd, sz);

    return fd;
}

void
resz()
{
    int32_t fd = alc_shm(w * h * 4); // argb

    pixl = mmap(0, w * h * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(shm, fd, w * h * 4);
    bfr = wl_shm_pool_create_buffer(pool, 0, w, h, w * 4, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
}

void
draw()
{

}

void
xrfc_conf(void* data, struct xdg_surface* xrfc, uint32_t ser)
{
    xdg_surface_ack_configure(xrfc, ser);
    if (!pixl) {
        resz();
    }
    draw();
    wl_surface_attach(srfc, bfr, 0, 0);
    wl_surface_damage_buffer(srfc, 0, 0, w, h); // Please update this
    wl_surface_commit(srfc); // The surface is good to go :D
}

struct xdg_surface_listener xrfc_list =
{
    .configure = xrfc_conf
};

void
top_conf(void* data, struct xdg_toplevel* top, int32_t w, int32_t h,
        struct wl_array* stat)
{

}

// To close the window
void
top_cls(void* data, struct xdg_toplevel* top)
{
    
}

struct xdg_toplevel_listener top_list =
{
    .configure = top_conf,
    .close = top_cls
};

void
sh_ping(void* data, struct xdg_wm_base* sh, uint32_t ser)
{
    xdg_wm_base_pong(sh, ser);
}

struct xdg_wm_base_listener sh_list =
{
    .ping = sh_ping
};

// Tells us what globals we'll need. Server-client consistent.
void
reg_glob(void* data, struct wl_registry* reg, uint32_t name, int8_t* intf,
        uint32_t v)
{
    if (!strcmp(intf, wl_compositor_interface.name))
    {
        comp = wl_registry_bind(reg, name, &wl_compositor_interface, 4);
    }
    else if (!strcmp(intf, wl_shm_interface.name))
    {
        shm = wl_registry_bind(reg, name, &wl_shm_interface, 1);
    }
    else if (!strcmp(intf, xdg_wm_base_interface.name))
    {
        sh = wl_registry_bind(reg, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(sh, &sh_list, 0);
    }
}

// Tells us when those globals no longer exist.
void
reg_glob_rem(void* data, struct wl_registry* reg, uint32_t name)
{
    
}

// Registry listener.
struct wl_registry_listener reg_list =
{
    .global = reg_glob,
    .global_remove = reg_glob_rem
};

int8_t
main()
{
    // Looks for an active Wayland server. Returns the connection to that server.
    struct wl_display* disp = wl_display_connect(0);
    if (!disp)
    {
        return -1;
    }

    struct wl_registry* reg = wl_display_get_registry(disp);
    wl_registry_add_listener(reg, &reg_list, 0);
    wl_display_roundtrip(disp); // Tells the server we're looking for data.

    srfc = wl_compositor_create_surface(comp);

    struct xdg_surface* xrfc = xdg_wm_base_get_xdg_surface(sh, srfc);
    xdg_surface_add_listener(xrfc, &xrfc_list, 0);
    top = xdg_surface_get_toplevel(xrfc);
    xdg_toplevel_add_listener(top, &top_list, 0);
    xdg_toplevel_set_title(top, "wayland client");
    wl_surface_commit(srfc);

    while (wl_display_dispatch(disp));

    if (bfr)
    {
        // Remember to destroy your buffer after you're done with it.
        wl_buffer_destroy(bfr);
    }
    // Remember to destroy your surface after you're done with it.
    wl_surface_destroy(srfc);
    // It's good practice to disconnect your display when you're done with it.
    wl_display_disconnect(disp);
    return 0;
}
