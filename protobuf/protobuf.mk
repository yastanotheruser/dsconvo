protofiles = $(wildcard *.proto)
pbheaders = $(addsuffix .pb.h, $(basename $(protofiles)))
pbcxxfiles = $(addsuffix .pb.cc, $(basename $(protofiles)))

.PHONY : all clean

all : $(pbheaders)
clean :
	rm -f $(pbcxxfiles) $(pbheaders)

$(pbheaders) : %.pb.h : %.proto
	protoc --cpp_out=. $^
