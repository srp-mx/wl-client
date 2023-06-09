#include <wayland-client.h>
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

// Tells us what globals we'll need. Server-client consistent.
void
reg_glob(void* data, struct wl_registry* reg, uint32_t name, int8_t* intf,
        uint32_t v)
{
    if (!strcmp(intf, wl_compositor_interface.name)) {
        comp = wl_registry_bind(reg, name, &wl_compositor_interface, 4);
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
