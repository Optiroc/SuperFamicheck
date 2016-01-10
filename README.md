#superfamicheck 0.2
program by david lindecrantz, optiroc@gmail.com (2016.01.10)  
Distributed under the terms of the [MIT license](./LICENSE)

superfamicheck is a simple command line utility that shows (and optionally fixes) header info for super nintendo / super famicom ROM images.

##building

provided you have a decent command line interface, (gnu)make and a c++11 capable compiler: simply run `make`.

##operation

	superfamicheck rom_file [options...]

the following options are available:

	-h, --help      show usage instructions
	-o, --out FILE  specify file to write (if -f)
	-f, --fix       fix header (checksum/title/size)

show info for file rom.sfc:
  
	superfamicheck rom.sfc

show info for file rom.sfc and fix header (replacing source file):

	superfamicheck rom.sfc -f

show info for file rom.sfc and write fixed ROM image to specified file:

	superfamicheck rom.sfc -f -o fixed.sfc

	
##acknowledgments

superfamicheck uses the following libraries:

* [ezOptionParser](http://ezoptionparser.sourceforge.net) by remik ziemlinski
