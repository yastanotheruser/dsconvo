protofiles = $(wildcard *.proto)
pbheaders = $(addsuffix .pb.h, $(basename $(protofiles)))
pbcxxfiles = $(addsuffix .pb.cc, $(basename $(protofiles)))

.PHONY : all clean

all : $(pbheaders) $(pbcxxfiles)
clean :
	rm -f $(pbheaders) $(pbcxxfiles)

$(pbheaders) $(pbcxxfiles) &: $(protofiles)
	protoc --cpp_out=. $^
