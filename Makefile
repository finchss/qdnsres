CC=clang
all:qdnsres
qdnsres: qdnsres.c
	$(CC) qdnsres.c -o qdnsres -Wall  -lpthread
