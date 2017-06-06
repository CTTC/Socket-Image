DEBUG= -g
OPENCV= -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video

PKG_CONF_CFLAG= `pkg-config --cflags opencv --libs opencv`
PKGCONF_LIBS= `pkg-config --libs opencv`

client:
	clear
	g++ $(PKG_CONF_CFLAG) -o client client.cpp -std=c++11 $(PKGCONF_LIBS)

server:
	clear
	g++ $(PKG_CONF_CFLAG) -o server server.cpp -std=c++11 $(PKGCONF_LIBS)
