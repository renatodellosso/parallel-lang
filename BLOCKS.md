Blocks are used for groups of expressions, such as the bodies of conditionals, loops, and functions. A block has its own scope.

Consider the following:
```
int a = 1; // 1
int b = 2; // 2

if (a == 1) { // 3
  b++; // 4
}

a = b; // 5
```
In this example, the condition cannot be evaluated until 1 is run. Similarly, 4 depends on both 2 and 3: b must be declared before it can be incremented, and the condition must be evaluated before we know if the increment should even run. 5 cannot run until either 3 evaluates to false or 4 executes.

**Option 1: Abstract blocks**
Blocks could be abstracted into single statements for dependency purposes. In this case, each block was consider its internal dependents and the block as a whole would depend on everything any instruction in the block depended on. In the above example, the entire block was depend on instructions 1 and 2. Likewise, 5 would depend on the entire block.

This option is easier to implement, but reduces performance. A function that accesses many resources could block a huge number of instructions.

**Option 2: Smart Blocks**
Inter-block dependencies are built as normal. If a conditional executes, dependencies are resolved as normal. If the conditional does not execute, it searches its contents for dependents and resolves them. In the example above, if the condition fails, the dependency 4 -> 5 is fulfilled by the conditional instruction.

Loop dependencies are routed through the loop condition. Consider an instruction in the body of a loop. Its dependents cannot be resolved until we know the loop is done running.

Here's an example:
```
int a = 1; // 1
while (a % 2 == 1) // 2
  a++; // 3
a = 5; // 4
```
3 clearly depends on 2, but, where 4 would ordinarily depend on 3, it actually depends on 2. Furthermore, the initial condition check depends on 1, but subsequent checks depend on 3.

There are a few options for functions. First, the body of a function could be copied into the bytecode where the call is, but this would massively bloat the bytecode.

The second option is more complicated. In the bytecode, the function call becomes an instruction that runs as soon as we know the function will be called. 