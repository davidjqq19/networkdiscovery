objs = main.o iface.o netbios.o arpreader.o

libs = -lm -lpthread -lpcre

opt = -Wall

install : $(objs)
	gcc -o networkdiscovery $(objs) $(libs) $(opt)
	rm *.o

main.o : main.c
	gcc -c main.c $(opt)

iface.o : iface.c
	gcc -c iface.c $(opt)

netbios.o : netbios.c
	gcc -c netbios.c $(opt)

arpreader.o : arpreader.c
	gcc -c arpreader.c $(opt)

clean: 
	rm networkdiscovery
