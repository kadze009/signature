# signature

`signature` is an application which was written on pure C++17. The app splits
*an input* file on blocks of selected size and computes a signature of each
block simultaneously (multithreading). The signatures are saved to *an output*
file.



## Usage

```
Usage:
    signature [KEYS]... <INPUT_FILE> <OUTPUT_FILE>

DESCRIPTION
    The application splits the input file on blocks of selected size and computes
    the signature of each block simultaneously. The signatures are saved to
    output file.

KEYS
    -h, --help
        Show this message

    --version
        Print version

    -v, --verbose LEVEL (default: WRN)
        Print information verbosly. Possible values: DBG, INF, WRN, ERR.

    -b, --block-size BLOCK_SIZE (default: 1M)
        The length of the block by which an input file will be splited. Supported
        suffixes: K=KiloByte, M=MegaByte, G=GigaByte. A number without suffix
        is interpreted as KiloBytes.

    -o, --option OPTION
        Set special option:
        * sign_algo=[crc32,md5] (default: md5)
            the signature algorithm
        * threads=NUM (default: as many threads as available)
            the integer number of threads for processing (must be more then 0)
        * log_file=<file path> (default: stdout)
            the log file path

EXAMPLES
    signature input.dat output.dat
    signature -b 32K input.dat output.dat -o threads=5
    signature --block-size 32K input.dat out.dat -o sign_algo=md5 -o threads=5
```



## The application architecture overview

There are some **singletone** entities in the application:
  * `Config` - a class, which processes, validates and stores input attributes.
  * `Logger` - a **thread local** class which provides functionalities for
    creation logging messages.
  * `LoggerManager` - a class which links messages from *Logger* and deferredly
    writes them.
  * `WorkerManager` - a class which creates, stores and handles results of
    *Workers* which compute signatures of the input file's blocks.
  * `PoolManager` - a class which creates and stores lists of pools which
    other application entities request.

A main work is processed by entries of class *Worker*. This class
   1. splits input file by blocks,
   2. calculates signature of a block,
   3. saves the signature and the block number,
   4. sends the result to the *WorkerManager*.

The class pairs (*Worker*, *WorkerManager*) and (*Logger*, *LoggerManager*)
works the same and their similar work is implemented in `IDeferedQueue`
class. The multithread handling is split by two parts:
   1. Get an item from a pool, fill it and send to a manager. The manager
      adds the item to the end of itself linked list (Compare-And-Swap paradigm).
   2. On the time, the manager handles batch of items from the head of its
      internal list to the pre-last node of the list or the selected number
      of list's nodes.



## TODO

1. Add APP's attribute options:
   - `read_buffer_size=BYTES`: initial value for read buffer;
   - `block_filler=CHAR`: the symbol which will be used for filling block if
     needed;
   - `final_stats=BOOL`: print some statistics at the end of execution (pools
     sizes, number of pools, execution time);
   - `runtime_stats=BOOL`: calculate and print Workers' statistics
     (microseconds for block processing)
2. **WARNING**: Implement CRC32 algorithm.

