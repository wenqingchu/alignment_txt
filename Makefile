OPENCV:=`pkg-config --cflags --libs opencv`



CC:=g++

CPPS:=alignment.cpp LP.cpp radon.cpp util.cpp

BIN:=alignment


main: $(CPPS)
	$(CC) -o $(BIN) $(CPPS)  -I$(OPENCV) -L/usr/local/cuda/lib64/ -lcufft -lnpps -lnppi -lnppc -lcudart -std=c++11


clean: 
	rm -f $(BIN) *.o
