all:
	gcc -O4 -o server main.c server.c config.c logger.c \
	-lcjson -lglib-2.0 -lgstrtspserver-1.0 -lgobject-2.0 -pthread \
	-I/usr/include/glib-2.0 \
	-I/usr/include/gstreamer-1.0 \
	-I/usr/lib/arm-linux-gnueabihf/glib-2.0/include