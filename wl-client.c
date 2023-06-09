#include <wayland-client.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

int8_t
main()
{
    // Looks for an active Wayland server. Returns the connection to that server.
    struct wl_display* disp = wl_display_connect(0);
    if (!disp)
        return -1;

    // It's good practice to disconnect your display when you're done with it.
    wl_display_disconnect(disp);
    return 0;
}
