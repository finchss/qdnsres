CC=clang
all:
	$(CC) qdnsres.c -o qdnsres -Wall  -lpthread
