##Which program is fastest? Is it always the fastest?
- malloc is the fastest at large scale. At 1,000,000 blocks it averaged 0.337s,
beating every other program. At small scale (10,000 blocks) it ties with alloca at
~0.010s.
- alloca is not always fastest despite using stack allocation, because it uses recursion to keep stack frames alive, with one recursive call per Node. At 1,000,000 blocks that means 1 million recursive calls, which adds significant overhead. You can see this in the memory usage: alloca used 202,756 KB at 1M blocks while malloc only used 93,324 KB. The recursion cost overtakes the stack allocation benefit at large scale.

##Which program is slowest? Is it always the slowest?
- List is consistently the slowest across all configurations. At 1,000,000 blocks it averaged 1.332s — nearly 4× slower than malloc.
  - The reason is that list performs two heap allocations per Node:
    - std::list::push_back allocates an internal list-node wrapper on the heap
    - The Node constructor triggers a second heap allocation for the std::vector buffer
    - Every extra allocation adds allocator overhead, increases heap fragmentation, and hurts cache performance. new is a close second (1.207s at 1M blocks) for the same reason — it also allocates the Node struct and the data bytes separately on the heap.

$$Was there a trend in program execution time based on the size of data in each Node? If so, what, and why?
- Yes, but only visible at larger block counts. At 10,000 blocks the times were flat across all node sizes because the dataset fits in CPU cache regardless of byte size. The trend becomes clear at higher block counts: As node data size increases, heap-based programs (list, new, malloc) slow down
because:
  - The data initialization loop (filling each byte) runs longer
  - Larger heap allocations increase cache pressure — more data scattered across memory means more cache misses during traversal
- alloca was largely unaffected by node size because all data lives on the stack, which is already in CPU cache. Whether you allocate 10 bytes or 4000 bytes via alloca, the memory is right there in the current stack frame.

##Was there a trend in program execution time based on the length of the block chain?
  - Yes, runtime scales linearly (O(n)) with NUM_BLOCKS for all programs.
  - Each additional block requires one allocation and one data-fill (both O(1)), and the hash traversal visits every node once (O(n) total). So doubling the blocks roughly doubles the runtime.
  - malloc scales the best — lowest per-node cost at every step. alloca's recursion overhead grows with block count, which hurts its scaling.
  - list scales the worst because two heap allocations per node compounds at large N.

##Consider heap breaks, what's noticeable? Does increasing the stack size affect the heap? Speculate on any similarities and differences in programs?
  - Every program showed exactly 1 brk() call at every block count (10k, 100k, 1,000,000):
  - The stack and heap are completely separate memory regions that grow toward each other from opposite ends of the address space. ulimit -s unlimited only raises the ceiling on how far the stack is allowed to grow downward — it says nothing to the heap and the heap allocator doesn't even know it happened.

##Considering either the malloc.cpp or alloca.cpp versions of the program, generate a diagram showing two Node. Include in the diagram the relationship of the head, tail, and Node next pointers. Show the size (in bytes) and structure of a Node that allocated six bytes of data include the bytes pointer, and indicate using an arrow which byte in the allocated memory it points to.

┌─────────┐
          head ─────►│  Node A │
                     ├─────────┴──────────────────────────────────┐
                     │  bytes*    │  8 bytes  │ ──────────────────┼──┐
                     │  numBytes  │  8 bytes  │  value = 6        │  │
                     │  next*     │  8 bytes  │ ──────────────────┼──┼──► Node B
                     └───────────────────────────────────────────-┘  │
                      sizeof(Node) = 24 bytes total                   │
                                                                      │
                                                                      ▼
                     ┌──────────────────────────────────────┐
                     │  Data Block (6 bytes, on heap/stack) │
                     │                                      │
                     │  byte[0] byte[1] byte[2] byte[3] byte[4] byte[5]
                     │  0x00    0x01    0x02    0x03    0x04    0x05
                     │    ▲
                     └────┼─────────────────────────────────┘
                          │
                          └── bytes* points here (byte[0])


                     ┌─────────┐
          tail ─────►│  Node B │
                     ├─────────┴──────────────────────────────────┐
                     │  bytes*    │  8 bytes  │ ──────────────────┼──► (its own data block)
                     │  numBytes  │  8 bytes  │  value = (varies) │
                     │  next*     │  8 bytes  │  nullptr          │
                     └────────────────────────────────────────────┘
                          next* == nullptr means this is the last node


  In malloc.cpp:
    Node struct (24 bytes) ── HEAP, allocated by malloc(sizeof(Node))
    Data block  (6 bytes)  ── HEAP, allocated by malloc(6) inside constructor

  In alloca.cpp:
    Node struct (24 bytes) ── STACK, allocated by alloca(sizeof(Node))
    Data block  (6 bytes)  ── STACK, allocated by alloca(6)

##There's an overhead to allocating memory, initializing it, and eventually processing (in our case, hashing it). For each program, were any of these tasks the same? Which one(s) were different?
  - The data initialization and the hash traversal are completely identical across all four programs
  - Only the allocation and deallocation strategies differ
  - list does 2 heap allocations per node (list wrapper + vector buffer) new and malloc do 1 heap allocation for the Node + 1 for the data = 2 total
  - alloca does 0 heap allocations — both the Node struct and data go on the stack
  - malloc uses placement new to separate memory acquisition from construction, requiring an explicit destructor call (node->~Node()) before free()

##As the size of data in a Node increases, does the significance of allocating the node increase or decrease?
  - As the size of data in a node increases,allocation overhead becomes relatively less significant. The initialization loop and hash traversal both grow linearly with byte count, so as nodes get larger, the time spent touching the data increasingly dominates and the time spent on the allocation call itself becomes a shrinking fraction of the total per-node work.