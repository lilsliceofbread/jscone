# jscone

my attempt at a json parser in c

resources used: [json.org's JSON flowchart](https://www.json.org/), [VoxelRift's video about parsing mathematical expressions](https://youtu.be/myZcNjKcVGw)

- single header

- creates an n-ary tree which can be searched through like a directory to find values

- supports c99, only requires libc

- prints json error line numbers (roughly)

- only parsing, no writing (yet)

- can only handle unicode up to 0xFFFF

- default uses malloc - can replace macro with your allocation function

## using it

include in your main file with:

```
#define JSCONE_IMPLEMENTATION // defines the functions in this file only
#include "jscone.h"
```

and then you can include jscone.h in any other source files

run example with:

```
cd example
make run
```

## testing

run tests with:

```
cd tests
make run
```

tests are created and run automatically using cursed macros (doesn't work on msvc)
