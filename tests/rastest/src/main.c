#include <am.h>
#include <klib.h>

#define TRAINING_LOOPS 4096
#define WARMUP_LOOPS   64

static inline uint64_t read_cycle(void) {
    uint64_t cycles;
    asm volatile ("rdcycle %0" : "=r"(cycles));
    return cycles;
}

void __attribute__((noinline)) empty_func(void) {
    asm volatile ("" : : : "memory");
}

void __attribute__((noinline)) ras_push_pop_test(int loops) {
    for (int i = 0; i < loops; i++) {
        empty_func();
    }
}

/*
 * Exercise the RISC-V RAS pop-and-push stack-update hint:
 *   jalr t0, 0(ra): rs1 = x1, rd = x5, pop x1 then push x5
 *   jalr ra, 0(t0): rs1 = x5, rd = x1, pop x5 then push x1
 *
 * Important distinction:
 *   This is still an indirect call-like instruction. The architectural target
 *   comes from rs1, so a frontend should not use the RAS top as this
 *   instruction's predicted target. The RAS hint only tells the return stack
 *   predictor how to update its internal stack state: pop the old top, then
 *   push the new link address produced by the call.
 *
 * Register roles inside the inline assembly:
 *   ra/x1:  initially points to coroutine B. Later it also carries B's
 *           continuation address, just like a coroutine link register.
 *   t0/x5:  initially points to coroutine A. Later it carries A's continuation
 *           address. x5 is the alternate RISC-V link register.
 *   t2:     loop counter. It is incremented only after control returns from B
 *           back to A, so it counts full A -> B -> A round trips.
 *
 * One loop iteration performs an A -> B -> A coroutine round trip, so the
 * normalized result printed by main is cycles per round trip.
 */
void __attribute__((noinline)) ras_popandpush_coroutine_test(int loops) {
    asm volatile(
        /*
         * The inline assembly overwrites ra to drive the coroutine jumps.
         * Save the caller's real return address first so this C function can
         * return normally after the benchmark loop finishes.
         */
        "addi sp, sp, -16\n\t"
        "sd ra, 8(sp)\n\t"

        /* t2 is the completed round-trip counter. */
        "li t2, 0\n\t"

        /*
         * Seed the two coroutine entry addresses:
         *   ra = label 2, coroutine B entry
         *   t0 = label 1, coroutine A entry
         *
         * After the first jalr, each register is overwritten with the address
         * following that jalr. From then on, ra and t0 hold continuation
         * addresses rather than the original entry addresses.
         */
        "la ra, 2f\n\t"
        "la t0, 1f\n\t"

        /*
         * Start execution in coroutine A. This plain jump is only bootstrap
         * control flow; the measured coroutine switching begins at label 1.
         */
        "j 1f\n\t"

        /*
         * Coroutine A:
         *   jalr t0, 0(ra)
         *     - jumps to the address in ra, which is coroutine B
         *     - writes A's continuation address, the addi below, into t0
         *     - uses rd=x5 and rs1=x1, which is the RAS pop-and-push case
         *
         * The jump target itself is not predicted from the RAS top. This
         * instruction is indirect-control flow, while the RAS action is only a
         * stack update: consume the previous top and push A's continuation.
         *
         * When B jumps back, execution resumes at the addi below. At that
         * point one full A -> B -> A round trip has completed.
         */
        ".p2align 3\n\t"
        "1:\n\t"
        "jalr t0, 0(ra)\n\t"
        "addi t2, t2, 1\n\t"

        /*
         * Continue until t2 reaches loops. Branching back to label 1 starts
         * another A -> B -> A exchange using the continuation addresses now
         * stored in ra and t0.
         */
        "blt t2, %0, 1b\n\t"
        "j 3f\n\t"

        /*
         * Coroutine B:
         *   jalr ra, 0(t0)
         *     - jumps to the address in t0, which is A's continuation
         *     - writes B's continuation address, the j below, into ra
         *     - uses rd=x1 and rs1=x5, the reverse RAS pop-and-push case
         *
         * As above, the RAS should update stack state for a future return; it
         * should not be used as the target provider for this jalr.
         *
         * The following j is normally reached only if control later resumes at
         * B's continuation address. It loops back to B's jalr so B can keep
         * returning control to A on every iteration.
         */
        ".p2align 3\n\t"
        "2:\n\t"
        "jalr ra, 0(t0)\n\t"
        "j 2b\n\t"

        /*
         * Restore the caller's return address and stack pointer. Without this,
         * the function would try to return through the coroutine value left in
         * ra instead of returning to the C caller.
         */
        "3:\n\t"
        "ld ra, 8(sp)\n\t"
        "addi sp, sp, 16\n\t"
        :
        : "r"(loops)
        : "t0", "t2", "ra", "memory"
    );
}

static uint64_t measure_push_pop(int loops) {
    uint64_t begin = read_cycle();
    ras_push_pop_test(loops);
    uint64_t end = read_cycle();
    return end - begin;
}

static uint64_t measure_popandpush(int loops) {
    uint64_t begin = read_cycle();
    ras_popandpush_coroutine_test(loops);
    uint64_t end = read_cycle();
    return end - begin;
}

int main(void) {
    ras_push_pop_test(WARMUP_LOOPS);
    ras_popandpush_coroutine_test(WARMUP_LOOPS);

    uint64_t push_pop_cycles = measure_push_pop(TRAINING_LOOPS);
    uint64_t popandpush_cycles = measure_popandpush(TRAINING_LOOPS);

    printf("RAS test loops: %d\n", TRAINING_LOOPS);
    printf("call/ret push+pop:      %lu cycles, %lu cycles/iter\n",
           (unsigned long)push_pop_cycles,
           (unsigned long)(push_pop_cycles / TRAINING_LOOPS));
    printf("jalr pop-and-push pair: %lu cycles, %lu cycles/iter\n",
           (unsigned long)popandpush_cycles,
           (unsigned long)(popandpush_cycles / TRAINING_LOOPS));

    return 0;
}
