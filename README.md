# signature

The program which splits the input file on blocks with the selected size and
computes the signature of the each block. The signatures is saved in output
file.


## Usage
```sh
signature [OPTIONS] <input file> <output file>

OPTIONS
	-b,--block-size (default: 1M)
		the split block size

	-o <special key>
		few special options:
		algo=<crc32 | md5>
```

