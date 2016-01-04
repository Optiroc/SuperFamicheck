#superfamicheck 0.1
program by david lindecrantz, optiroc@gmail.com (2016.01.03)  
Distributed under the terms of the [MIT license](./LICENSE)

superfamicheck is a simple command line utility that shows (and optionally fixes) header info for super nintendo / super famicom ROM images.


##operation

	superfamicheck [options...]

the following options are available:

	-h, --help      show usage instructions
	-i, --in FILE   specify file to read
	-o, --out FILE  specify file to write (if -f)
	-f, --fix       fix header (checksum/title/size)

show info for file rom.sfc:
  
	superfamicheck -i rom.sfc

show info for file rom.sfc and fix bad fields (in-place):

	superfamicheck -i rom.sfc -f

show info for file rom.sfc and write fixed fix ROM image to other file:

	superfamicheck -i rom.sfc -o fixed.sfc -f

	
##acknowledgments

superfamicheck uses the following libraries:

* [ezOptionParser](http://ezoptionparser.sourceforge.net) by remik ziemlinski
