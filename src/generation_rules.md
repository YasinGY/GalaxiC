<h1>Registers</h1>
<br>
<table>
  <tr>
    <th>Register</th>
    <th>Name</th>
    <th>Purpose</th>
  </tr>
  <tr>
    <td>RAX</td>
    <td>Accumulator</td>
    <td>Return values from functions, arithmetic operations (e.g., multiplication, division).</td>
  </tr>
  <tr>
    <td>RBX</td>
    <td>Base Register</td>
    <td>General-purpose, often preserved across function calls.</td>
  </tr>
  <tr>
    <td>RCX</td>
    <td>Counter</td>
    <td>Used for loop counters, string operations, and function arguments in Windows x64 calling convention.</td>
  </tr>
  <tr>
    <td>RDX</td>
    <td>Data Register</td>
    <td>Secondary register for arithmetic, also for passing function arguments.</td>
  </tr>
  <tr>
    <td>RSI</td>
    <td>Source Index</td>
    <td>Used in memory operations, such as memory copying.</td>
  </tr>
  <tr>
    <td>RDI</td>
    <td>Destination Index</td>
    <td>Used in memory or string operations and for passing arguments in Linux calling convention.</td>
  </tr>
  <tr>
    <td>RBP</td>
    <td>Base Pointer</td>
    <td>Points to the start of the current stack frame, used for managing local variables.</td>
  </tr>
  <tr>
    <td>RSP</td>
    <td>Stack Pointer</td>
    <td>Points to the top of the stack, essential for function calls and return addresses.</td>
  </tr>
  <tr>
    <td>R8-R11</td>
    <td>Extended Registers</td>
    <td>General-purpose registers used for additional function arguments (System V calling convention).</td>
  </tr>
  <tr>
    <td>R12-R15</td>
    <td>Extended Registers</td>
    <td>General-purpose registers often used for holding numbers and preserved across function calls (callee-saved).</td>
  </tr>
  <tr>
    <td>RIP</td>
    <td>Instruction Pointer</td>
    <td>Points to the next instruction to be executed in the program.</td>
  </tr>
</table>

<h2>Labels</h2>
<br>
<table>
  <tr>
    <th>Label Name</th>
    <th>Purpose</th>
  </tr>
  <tr>
    <td>MainLabel</td>
    <td>The main label where control returns after other labels are done. It acts like the main thread of the program.</td>
  </tr>
  <tr>
    <td>BoolLabel</td>
    <td>Handles boolean calculations, such as comparisons (e.g., checking if a value equals another).</td>
  </tr>
  <tr>
    <td>IfLabel</td>
    <td>Used to evaluate conditions for `if` statements. You can either place the comparison here or use a separate label for the scope.</td>
  </tr>
  <tr>
    <td>LoopLabel</td>
    <td>Handles loops (e.g., `while` or `for` loops) and jumps back to check the condition.</td>
  </tr>
</table>

<h1>Function Arguments</h1>
<br>

<table>
  <tr>
    <th>Parameter #</th>
    <th>Register (Integer/Pointer)</th>
    <th>Register (Floating Point)</th>
  </tr>
  <tr>
    <td>1</td>
    <td>RCX</td>
    <td>XMM0</td>
  </tr>
  <tr>
    <td>2</td>
    <td>RDX</td>
    <td>XMM1</td>
  </tr>
  <tr>
    <td>3</td>
    <td>R8</td>
    <td>XMM2</td>
  </tr>
  <tr>
    <td>4</td>
    <td>R9</td>
    <td>XMM3</td>
  </tr>
  <tr>
    <td>Additional Parameters</td>
    <td colspan="2">Passed on the stack</td>
  </tr>
</table>
