# jscone

WORK IN PROGRESS

TODO:

- create unit tests

- finish parser (parse_number)

- implement find function (paths)

- pool memory instead of many mallocs

- functionise/simplify

- optionally separate c file

- optimise?

my attempt a json parser in c

resources used: [json.org's JSON flowchart](https://www.json.org/), [VoxelRift's video about parsing mathematical expressions](https://youtu.be/myZcNjKcVGw)

- single header

- only parsing, no writing (yet)

- creates n-ary tree of values

run example with:

```
cd example
chmod +x ./run.sh
./run.sh
```
