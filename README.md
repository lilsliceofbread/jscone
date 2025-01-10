# jscone

WORK IN PROGRESS

TODO:

- implement find function (paths)

- use json tester to find errors

- pool memory instead of many mallocs

- functionise/simplify

- optionally separate c file

my bad attempt a json parser in c

resources used: [json.org's JSON flowchart](https://www.json.org/), [VoxelRift's video about parsing mathematical expressions](https://youtu.be/myZcNjKcVGw)

- single header

- only parsing, no writing (yet)

- creates n-ary tree of values

- can only handle unicode up to 0xFFFF

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
