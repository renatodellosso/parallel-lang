# Goals

Build a language that optimizes code by automatically running sections of it in parallel.

For example, if you have the following code:

```
let arr = [1, 2, 3, 4, 5];
mergeSort(arr[0..2]);
mergeSort(arr[3..5]);
```

The language should automatically detect that the two `mergeSort` calls can be run in parallel and execute them concurrently, improving performance without the user having to explicitly manage threads or synchronization.

# Implementation

## Graph Representation

A program can be represented as a directed acyclic graph (DAG). Each vertex is one or multiple operations; an edge from A to B means that B depends on A.

Consider the following code:

```
let a = 1;
let b = 2;

let c = a + b;

let d = a - b;

let e = c * d;
```

The graph would look like this:

```
a -> c, d
b -> c, d
c -> e
d -> e
```

Vertex C can only be executed once vertices A and B have been executed.

## High-Level Overview

1. **Compilation (code -> instructions)**: Source code is parsed and converted into instructions.
1. **Compilation (instructions -> graph)**: Instructions are analyzed to build a DAG representing the dependencies between operations. The graph is saved as a file.
1. **Interpretation**: The graph is loaded from a file and traversed.

## Compilation

The compilation process consists of two steps: converting the source code to instructions, and then converting those instructions to a graph.

### Code to Instructions

An instruction is the representation of an operation in a state that it can be executed. For example, the code `let c = a + b;` would be converted to an instruction that represents the addition operation, along with the operands `a` and `b`.

This compilation step works as follows:

1. The source code is converted to tokens.
1. The tokens are parsed to build instructions. Syntax errors are detected during this step.
1. The instructions are validated to ensure that they are semantically correct (e.g., variables are declared before use, types are consistent, etc.).

### Instructions to Graph

Each function is analyzed to build a DAG representing the dependencies between operations. This is done by examining the operands of each instruction and determining which instructions depend on the results of others. For example, if an instruction uses the result of another instruction, an edge is created from the instruction that produces the result to the instruction that consumes it.

This step proceeds as follows:

1. Each instruction becomes a vertex in the graph.
1. Each instruction's operands are analyzed to determine which resources it depends on. Resources include variables, I/O, etc.
1. The order of instructions and their dependencies are used to convert resource dependencies into instruction dependencies. These dependencies are represented as edges in the graph.
1. The graph is saved to a file for later use during interpretation.

Consider the following code:

```
let a = 1;
let b = 2;

if (a == b) {
  notify();
}

a = fibonacci(a);
b = tribonacci(b);

let c = a + b;
return c;
```

The vertices might look like this:

```
=== Vertex 1 ===
let a = 1;
let b = 2;

=== Vertex 2 ===
if (a == b) {
  notify();
}

=== Vertex 3 ===
a = fibonacci(a);

=== Vertex 4 ===
b = tribonacci(b);

=== Vertex 5 ===
let c = a + b;
return c;
```

The edges would be:

```
1 -> 2, 3, 4
3 -> 5
4 -> 5
```

## Interpretation

The interpeter maintains a queue of vertices to execute and a pool of threads to execute them. When started, the interpreter loads the graph from file. It finds the main function, and adds it to the queue.

### Interpreter Threads

Each interpreter thread continuously pulls the first vertex from the queue. If not all its dependencies are fulfilled, it is pushed back onto the queue. Otherwise, the thread executes the vertex's instructions. Once all statements in the vertex have been executed, the thread pushes the vertices' children onto the queue.