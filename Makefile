# -*- MakeFile -*-

all: UKSU_server UKSU_client
UKSU_server: UKSU_server.c
	gcc UKSU_server.c -o UKSU_server -Wall
UKSU_client:UKSU_client.c
	gcc UKSU_client.c -o UKSU_client -Wall

clean:
	rm -f UKSU_client UKSU_server