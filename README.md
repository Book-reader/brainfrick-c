# A fast & powerful brainf**k interpreter written in c

## Building
```sh
gcc main.c [compile options] -o brainfrick
```

## Compile options
 - `-Dlargecells`, Makes each cell 16 bit instead of the default byte. Can break some programs
 - `-Dverbose`, Prints basically everything. Prints all output at once at the end
 - `-Dgrowablemem`, Gives the program an unlimited amount of cells instead of the default 30,000


## Running
```sh
./brainfrick [path to brainf**k file]
```

or to immediatly pass input into a brainf**k file

```sh
echo "My Input" | ./brainfrick [file]
```

### Examples

 - running the included bubble sort:
    ```sh
    $ echo "Hello, World!" | ./brainfrick files/bsort.b
    Hello, World!<RET><CTRL-D>
    !,HWdellloor
    ```
 - passing text to included bubble sort:
    ```sh
    $ echo "Hello, World!" | ./brainfrick files/bsort.b
    !,HWdellloor
    ```
 - passing program + args to included bf interpreter:
    ```sh
    $ echo $(cat file/bsort.b) "!" "Hello, World!" | ./brainfrick files/bf-interpreter2.b
    !,HWdellloor
    ```

## Sample brainf**k files

All the included files belong to their respective authors.
 **Please note that some programs that can take unlimited input (the interpreters, bsort, etc) require Ctrl-D to be pressed after typing input and pressing enter**
 - `99bottles.bf` - 99 Bottles of beer on the wall. 99 Bottles of beer.
 - `196.b` - The 196 algorithm. checks if the entered number is a lychrel number
 - `bee-movie.b` - prints the bee movie script
 - `bf-interpreter(1,2,3).b` - 3 different brainf**k interpreters writter in bf. They have varying levels of speed for different tasks. after entering the program, add a ! (required) then write any input for the program. the input can also be piped in
 - `bf-standard-compliance-test.bf` - tests whether the interpreter is fully bf-compliant
 - `bf-standard-compliance-test-for-recursive.bf` - same as above, but with any ! in the comments removed as `bf-interpreter(1,2,3).b` cannot handle them (they are not fully compliant to the spec as they use ! to seperate the program and the inputs)
 - `bsort.b` - bubble sorts inputs (by ascii value)
 - `dbf2c.b` - converts valid brainf**k code into c (in the most literal way possible)
 - `e.b` - computes the transcendental number e
 - `fib.b` - prints the fibonacci sequence
 - `hanoi.b` - completes the tower of hanoi puzzle (a great benchmark)
 - `hello_world.b` - prints "Hello, World!"
 - `life.b` - conway's game of life
 - `LostKingdom.b` - an entire game? how. an "Enchanced Brainfuck Edition of the original BBC BASIC game"
 - `more_memory.b` - eats all your memory if you used `-Dgrowablemem`
 - `numwarp.b` - makes numbers look cool
 - `PI16.BF` - calculate pi (number of digits changed inside file)
 - `PRIME.BF` - computes prime numbers
 - `random.b`- pseudo random using Rule 30 automaton
 - `sierpinski.b` - Shows an ASCII representation of the Sierpinski triangle
 - `text-to-bf.b` - turns text into brainf**k code (poorly).