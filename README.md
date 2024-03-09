# superfamicheck
superfamicheck is a simple command line utility that shows (and optionally fixes) header info for super nintendo / super famicom ROM images.

superfamicheck is programmed by David Lindecrantz and distributed under the terms of the [MIT license](./LICENSE).


## building
in a unix-like environment simply `make` the binary. on windows use cmake to generate a build environment.

## operation

	superfamicheck rom_file [options...]

the following options are available:

	-h, --help        show usage instructions
	-o, --out FILE    specify file to write (if -f)
	-f, --fix         fix header (checksum/title/size)
	-s, --semisilent  silent operation (unless issues found)
	-S, --silent      silent operation

show info for file rom.sfc:
  
	superfamicheck rom.sfc

show info for file rom.sfc and fix header (replacing source file):

	superfamicheck rom.sfc -f

show info for file rom.sfc and write fixed ROM image to specified file:

	superfamicheck rom.sfc -f -o fixed.sfc

	
## acknowledgments

superfamicheck uses the following libraries:

* [ezOptionParser](http://ezoptionparser.sourceforge.net) by remik ziemlinski
