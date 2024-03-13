SOURCEDIR = src
CC = gcc-8
CFLAGS =  -Wall -std=c99 -Wextra
CCFLAGS = -lpthread -lbcm2835 -ljson-c -lm -lssl -lcrypto
OBJECTS = ssltransaction.o fileparser.o calcmetrics.o AD-DA-WS-RPI.o
LDFLAGS='-L/usr/local/opt/openssl/lib' 
CPPFLAGS='-I/usr/local/opt/openssl/include'

NILMAPP = nilm
UPDATEAPP = updater

all: $(UPDATEAPP) $(NILMAPP)

ssltransaction.o: $(SOURCEDIR)/ssltransaction.c $(INCLUDESDIR)
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/ssltransaction.c

fileparser.o: $(SOURCEDIR)/fileparser.c $(INCLUDESDIR)
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/fileparser.c

calcmetrics.o: $(SOURCEDIR)/calcmetrics.c $(INCLUDESDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SOURCEDIR)/calcmetrics.c

AD-DA-WS-RPI.o: $(SOURCEDIR)/AD-DA-WS-RPI.c $(INCLUDESDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) -c $(SOURCEDIR)/AD-DA-WS-RPI.c

$(UPDATEAPP): $(INCLUDESDIR)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(CPPFLAGS) -o $(UPDATEAPP) $(CFLAGS) $(SOURCEDIR)/$(UPDATEAPP).c

$(NILMAPP): $(OBJECTS) $(INCLUDESDIR)
	$(CC) $(OBJECTS) $(CCFLAGS) $(LDFLAGS) $(CPPFLAGS) -o $(NILMAPP) $(CFLAGS) $(SOURCEDIR)/$(NILMAPP).c
	$(RM) -f *.o
clean:
	$(RM) -f $(NILMAPP) $(UPDATEAPP)  *.o