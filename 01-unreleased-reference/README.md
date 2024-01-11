## Compile eBPF programs

```bash
clang-14 -O2 -g  -Wall -Werror -target bpf -c bpf_flawed.c -I ../
clang-14 -O2 -g  -Wall -Werror -target bpf -c bpf_corrected.c -I ../
```

These should generate bpf object files bpf_corrected.o and bpf_flawed.o

```bash
ls
bpf_corrected.c  bpf_corrected.o  bpf_flawed.c  bpf_flawed.o
```

## Load programs into kernel

Using ```bpftool```, we'll try to load into kernel.
At this point verifier will run its checks on them.

// You'll need root permission


First try to load flawed program into the kernel.
```bash
bpftool prog load bpf_flawed.o /sys/fs/bpf/flawed
```
It should fail to load into kernel and output following in the response:

```bash
libbpf: prog 'sched_process_exec': failed to load: -22
libbpf: failed to load object 'bpf_flawed.o'
Error: failed to load object file
```
The log line we're interested is this.
```bash
Unreleased reference id=3 alloc_insn=10
```

Let's look at the 10th instruction
```bash
10: (85) call bpf_ringbuf_reserve#131         ; R0=ringbuf_mem_or_null(id=3,ref_obj_id=3,off=0,imm=0) refs=3
```
On the right side, verifier internals are logged.
A reference to a slot on ringbuf represented with id 3. 

Later on odd number check is made on instruction 13. And program continues in the basic block that exits.
```bash
13: (57) r1 &= 1                      ; R1_w=scalar(umax=1,var_off=(0x0; 0x1)) refs=3
14: (15) if r1 == 0x0 goto pc+4 19: R0=ringbuf_mem(ref_obj_id=3,off=0,imm=0) R1_w=0 R6=scalar(id=4,umax=4294967295,var_off=(0x0; 0xffffffff)) R10=fp0 refs=3
```

Let's look at the next instruction.
```bash
19: (b7) r0 = 0                       ; R0_w=0 refs=3
20: (95) exit
```
Before the exit, program still has reference to object with id 3, that we get in result of bpf_ringbuf_reserve call. That's why verifier stops us from loading the program into the kernel.

We need to release the reference, that's what we'll do in the corrected program.

```c
    // Just for the demonstration, we'll only submit an event for pids that end with odd numbers
    if (pid % 2 == 0){
        // Release reserved ringbuf location
        bpf_ringbuf_discard(e,0);
        return 0;
    }
```

We'll add debug option to see verifier logs on the corrected version.
```bash
bpftool --debug prog load bpf_corrected.o /sys/fs/bpf/corrected
```

Again, we get a reference to a slot on ringbuf on the same instruction.
```bash
10: (85) call bpf_ringbuf_reserve#131
last_idx 10 first_idx 0
regs=4 stack=0 before 9: (b7) r3 = 0
regs=4 stack=0 before 8: (b7) r2 = 8
11: R0=ringbuf_mem_or_null(id=3,ref_obj_id=3,off=0,imm=0) refs=3
```

Pid odd number check and call to ```bpf_ringbuf_discard```. Note that there are no refs left before exit instruction this time.
```bash
13: (57) r1 &= 1                      ; R1_w=scalar(umax=1,var_off=(0x0; 0x1)) refs=3
14: (55) if r1 != 0x0 goto pc+4       ; R1_w=0 refs=3

17: (85) call bpf_ringbuf_discard#133         ;

23: (b7) r0 = 0                       ; R0_w=0
24: (95) exit
```

For verifier logs:
bpftool --debug prog load bpf_corrected.o /sys/fs/bpf/corrected

// To check bpf instructions, and see what changes
llvm-objdump -S -r bpf_flawed.o > flawed
_inst.txt

llvm-objdump -S -r bpf_corrected.o > corrected
_inst.txt

