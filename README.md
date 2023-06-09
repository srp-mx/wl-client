# Cliente de Wayland de juguete.

Incluye `xdg-shell-client-protocol.h` con
```
wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
```

Incluye `xdg-shell-protocol.c` con
```
wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-protocol.c
```
