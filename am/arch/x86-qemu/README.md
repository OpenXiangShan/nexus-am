# x86-qemu

The reference (full) AbstractMachine implementation.

## AM outputs

* `_putc` prints to COM1.
* Trace data (packed `struct _TraceEvent`s) goes to COM2.

QEMU captures the outputs by setting `-serial [chardev] -serial [chardev2]`. Can be `stdio`, `file`, etc.
