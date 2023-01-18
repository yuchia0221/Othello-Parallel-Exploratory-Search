## Parallel Exploratory Search with Cilk Plus

#### Table of Contents

-   [Folder Structure](#folder-structure)
-   [Commands](#commands)

### Folder Structure

    .
    ├── examples                # Input Examples
    │   └── *.txt               # `c`: computer player, `h`: human player, `integer`: search depth
    ├── default_input           # Default Input File
    ├── othello-serial.cpp      # Serial Version with Alpha-Beta Pruning
    ├── othello.cpp             # Parallelized Version with Negamax
    ├── screen_input            # Default Screen Input File
    ├── Makefile                # Recipes for building and running your program
    └── README.md

### Commands

-   submit.sbatch:
    > a script that you can use to launch a batch job that will execute a series of tests on 1..16 cores on a compute node.

```bash
# you can modify this script to run your code multiple times with different
# lookahead depths using cilkview to collect scalability and performance metrics.
sbatch < submit.sbatch
```

Makefile:

> Makefile that includes recipes for building and running your program

```bash
make                # builds your code
make runp           # runs a parallel version of your code on W workers
make runs           # runs a serial version of your code on one worker
make screen         # runs your parallel code with cilkscreen
make view           # runs your parallel code with cilkview
make run-hpc        # creates a HPCToolkit database for performance measurements
make clean          # removes all executable files
make clean-hpc      # removes all HPCToolkit-related files
```
