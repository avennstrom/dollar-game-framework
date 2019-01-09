# dollar-game

A framework for creating custom solvers for The Dollar Game. Numberphile has a [great video](https://youtu.be/U33dsEcKgeQ) describing the concept of the game. It's also where I got the inspiration for this project.

## Building

### Windows

  1. Download [Premake](https://github.com/premake/premake-core)
  2. Place `premake5.exe` in the repository root
  3. Run `.\premake5.exe vs2017`
  4. Open generated solution in Visual Studio

### Linux

  1. Download [Premake](https://github.com/premake/premake-core)
  2. Place `premake5` in the repository root
  3. Run `./premake5 gmake`
  4. `make config=release_linux` or `make config=debug_linux`
  
## Usage

`.\DollarGame.exe --generator Star --solvers TakePoorest GiveRichest --graph-size 1000 --iterations 10 `

Example output:

```
| TakePoorest | GiveRichest |
|        1689 |        4999 |
|        1948 |        5000 |
|        1741 |        4993 |
|        1788 |        4998 |
|        1920 |        5002 |
|        1661 |        4994 |
|        1588 |        4994 |
|        1763 |        4990 |
|        1736 |        5005 |
|        1830 |        4997 |

#
# AVERAGE NUM MOVES
#

| TakePoorest | GiveRichest |
|     1766.40 |     4997.20 |
```

## Solvers

The goal of a solver is to solve the game (duh). The following code is a solver stub.

```c++
#include "../SolverCommon.hpp"

SOLVER_NAME("MySolver");
SOLVER_DESCRIPTION("My very own solver.");
SOLVER_FUNC(SolverContext& ctx)
{
    while (!ctx.isSolved())
    {
        // figure out the next best move here
    }
}
```

Follow these steps if you want to implement your own solver:
  
  1. Create a new file `MySolver.cpp` and place it in `src/solvers/`.
  2. Run `premake5.exe` to generate project files for the solver.
  3. Compile.
  
## Generators

Different generators can be used to generate graphs for these solvers to solve. Generators are defined in a similar manner. Here's a generator stub.

```c++
#include "../GeneratorCommon.hpp"

GENERATOR_NAME("MyGenerator");
GENERATOR_DESCRIPTION("My very own generator.");
GENERATOR_FUNC(GeneratorContext& ctx, const GeneratorParams& params)
{
    // generate a very cool graph here
    ctx.graph().init(...);
}
```

Follow these steps if you want to implement your own generator:

  1. Create a new file `MyGenerator.cpp` and place it in `src/generators/`.
  2. Run `premake5.exe` to generate project files for the generator.
  3. Compile.
