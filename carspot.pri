
carmode_desktop.files = $${TARGET}_harmattan.desktop.carmode
carmode_desktop.path  = /opt/carmode/share/desktop
INSTALLS += carmode_desktop

upnp_conf.files = carspot.conf carspot.xml
upnp_conf.path  = /opt/carmode/share/upnpapps
INSTALLS += upnp_conf

upnp_icon.files = carspot.svg
upnp_icon.path  = /opt/carmode/share/icons
INSTALLS += upnp_icon

