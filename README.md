# signature

`signature` is an application which was written on pure C++17. The app splits
the input file on blocks of selected size and computes the signature of each
block simultaneously. The signatures are saved to output file.



## Usage

```
Usage:
    signature [KEYS]... <INPUT_FILE> <OUTPUT_FILE>

DESCRIPTION
    The application splits the input file on blocks with a selected size and
    computes the signature of the each block. The signatures are saved in
    output file.

KEYS
    -h, --help
        this message

    --version
        Print version

    -v, --verbose LEVEL (default: WRN)
        Print information verbosly. Possible values: DBG, INF, WRN, ERR.

    -b, --block-size BLOCK_SIZE (default: 1M)
        the size of block on which input file is split. The value supports
        suffixes: K=Kilobyte, M=Megabyte, G=Gigabyte. A number without suffix
        is KiloBytes.

    -o, --option OPTION
        set special option:
        * sign_algo=[crc32,md5] (default: md5)
            signature algorithm
        * threads=NUM (default: as many threads as available)
            integer number of threads for processing
            (at least 2: one -- manager, others -- workers)
        * log_file=<file path> (default: stdout)
            the log file path
        * log_batch_size=<number> (default: 100)
            the integer number of log messages for writing in async mode during
            multithread execution

EXAMPLES
    signature input.dat output.dat
    signature -b 32K input.dat output.dat -o threads=5
    signature --block-size 32K input.dat out.dat -o sign_algo=md5 -o threads=5
```



## The application architecture overview

There are some **singletone** entities in the application:
  * `Config` - a class, which parses input attributes and contains common
    options.
  * `Logger` - a **thread local** class, which provide functionalities for
    creation logging messages.
  * `LoggerManager` - a class, which links messages from *Logger* and deferred
    writes them.
  * `WorkerManager` - a class, which creates, stores and handles results of
    *Workers*.
  * `PoolManager` - a class, which creates and stores lists of pools which
    other application entities request.

A main work is processed by entries of class *Worker*. This class
   1. splits input file by blocks,
   2. calculates signature of block,
   3. saves the signature and block number,
   4. sends the result to the *WorkerManager*.

The class pairs (*Worker*, *WorkerManager*) and (*Logger*, *LoggerManager*)
works the same, and their similar work is implemented in `IDeferedQueue`
class. The multithread handling is split by two parts:
   1. Get an item from a pool, fill it and send to a manager. The manager
      adds the item to the end of itself linked list (Compare-And-Swap paradigm).
   2. On the time, the manager handles batch of items from the head of its
      internal list to the pre-last node of the list or the selected number
      of list's nodes.



## TODO

1. Change `FileReader` and `FileWriter` classes by `std::ofstream` and
   `std::ifstream`.
2. Add APP's attribute options:
   - `read_buffer_size=BYTES`: initial value for read buffer;
   - `block_filler=CHAR`: the symbol which will be used for filling block if
     needed;
   - `final_stats=BOOL`: print some statistics at the end of execution (pools
     sizes, number of pools, execution time);
   - `runtime_stats=BOOL`: calculate and print Workers' statistics
     (microseconds for block processing)
3. **WARNING**: Implement CRC32 algorithm.

