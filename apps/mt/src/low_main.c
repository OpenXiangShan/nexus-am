#include <mt_common.h>

#define MAILBOX_ROUNDS 32
#define ATOMIC_ITERS 4096
#define LOCK_ITERS 128

enum sample_id {
  SAMPLE_HELP = 0,
  SAMPLE_SPLIT,
  SAMPLE_MAILBOX,
  SAMPLE_ATOMIC,
  SAMPLE_LOCK,
  SAMPLE_INVALID,
};

static volatile intptr_t tls_values[NR_HARTS] = {0};

static volatile intptr_t mailbox_state = 0;
static volatile intptr_t mailbox_payload = 0;
static volatile intptr_t mailbox_reply = 0;

static volatile intptr_t plain_counter = 0;
static volatile intptr_t atomic_counter = 0;

static volatile intptr_t mutex_word = 0;
static volatile intptr_t inside_critical = 0;
static volatile intptr_t protected_counter = 0;
static volatile intptr_t trace_owner[LOCK_ITERS * NR_HARTS] = {0};

static __thread intptr_t tls_slot = 0;

static enum sample_id parse_sample(const char *args) {
  if (args == NULL || args[0] == '\0' || strcmp(args, "h") == 0 || strcmp(args, "help") == 0) {
    return SAMPLE_HELP;
  }
  if (strcmp(args, "1") == 0 || strcmp(args, "s") == 0 || strcmp(args, "split") == 0) {
    return SAMPLE_SPLIT;
  }
  if (strcmp(args, "2") == 0 || strcmp(args, "m") == 0 || strcmp(args, "mailbox") == 0) {
    return SAMPLE_MAILBOX;
  }
  if (strcmp(args, "3") == 0 || strcmp(args, "a") == 0 || strcmp(args, "atomic") == 0) {
    return SAMPLE_ATOMIC;
  }
  if (strcmp(args, "4") == 0 || strcmp(args, "l") == 0 || strcmp(args, "lock") == 0) {
    return SAMPLE_LOCK;
  }
  return SAMPLE_INVALID;
}

static void print_usage(void) {
  printf("Low-pressure multi-hart samples:\n");
  printf("  build: make low ARCH=riscv64-xs-dual LOW_MAINARGS=<sample>\n");
  printf("  1 | split   : read mhartid in main and verify per-hart TLS\n");
  printf("  2 | mailbox : two-hart ping-pong over shared memory\n");
  printf("  3 | atomic  : compare raw shared increments with atomic_add\n");
  printf("  4 | lock    : guard a critical section with a spin lock\n");
}

static void run_split_sample(int hartid) {
  const char *sample = "split";

  tls_slot = 0x100 + hartid;
  tls_values[hartid] = tls_slot;
  fence_rw();

  if (hartid == 0) {
    atomic_printf("[%s] hart 0 boots hart 1 and publishes tls=%ld\n", sample, tls_slot);
  } else if (hartid == 1) {
    atomic_printf("[%s] hart 1 joins later and publishes tls=%ld\n", sample, tls_slot);
  } else {
    fail_now(sample, hartid, "unexpected hart");
  }

  _barrier();

  if (hartid == 0) {
    expect(tls_values[0] == 0x100, sample, hartid, "hart 0 TLS value mismatch");
    expect(tls_values[1] == 0x101, sample, hartid, "hart 1 TLS value mismatch");
    atomic_printf("[%s] tls values observed: hart0=%ld hart1=%ld\n",
        sample, tls_values[0], tls_values[1]);
  }

  finish_case(sample, hartid);
}

static void run_mailbox_sample(int hartid) {
  const char *sample = "mailbox";

  if (hartid == 0) {
    for (intptr_t round = 0; round < MAILBOX_ROUNDS; round++) {
      intptr_t payload = 0x1000 + round * 7;
      mailbox_payload = payload;
      fence_rw();
      mailbox_state = 1;

      while (mailbox_state != 2) {
        cpu_relax();
      }
      fence_rw();

      expect(mailbox_reply == (payload ^ 0x5a5a), sample, hartid, "unexpected reply payload");
      fence_rw();
      mailbox_state = 0;
      fence_rw();
    }
  } else if (hartid == 1) {
    for (intptr_t round = 0; round < MAILBOX_ROUNDS; round++) {
      intptr_t expected_payload = 0x1000 + round * 7;
      while (mailbox_state != 1) {
        cpu_relax();
      }
      fence_rw();

      expect(mailbox_payload == expected_payload, sample, hartid, "mailbox payload mismatch");
      busy_delay(32);
      mailbox_reply = mailbox_payload ^ 0x5a5a;
      fence_rw();
      mailbox_state = 2;

      while (mailbox_state != 0) {
        cpu_relax();
      }
    }
  } else {
    fail_now(sample, hartid, "unexpected hart");
  }

  if (hartid == 0) {
    atomic_printf("[%s] completed %d ping-pong rounds\n", sample, MAILBOX_ROUNDS);
  }

  finish_case(sample, hartid);
}

static void run_atomic_sample(int hartid) {
  const char *sample = "atomic";

  for (int i = 0; i < ATOMIC_ITERS; i++) {
    intptr_t snapshot = plain_counter;
    busy_delay(8 + hartid);
    plain_counter = snapshot + 1;
    _atomic_add(&atomic_counter, 1);
  }

  _barrier();

  if (hartid == 0) {
    intptr_t expected = NR_HARTS * ATOMIC_ITERS;
    expect(atomic_counter == expected, sample, hartid, "atomic counter mismatch");
    expect(plain_counter > 0 && plain_counter <= expected, sample, hartid, "plain counter out of range");
    atomic_printf("[%s] plain=%ld atomic=%ld expected=%ld\n",
        sample, plain_counter, atomic_counter, expected);
  }

  finish_case(sample, hartid);
}

static void run_lock_sample(int hartid) {
  const char *sample = "lock";

  for (int i = 0; i < LOCK_ITERS; i++) {
    spin_lock(&mutex_word);

    expect(inside_critical == 0, sample, hartid, "critical section entered concurrently");
    inside_critical = hartid + 1;

    intptr_t slot = protected_counter;
    expect(slot < LOCK_ITERS * NR_HARTS, sample, hartid, "trace buffer overflow");
    busy_delay(24);
    protected_counter = slot + 1;
    trace_owner[slot] = hartid;

    expect(inside_critical == hartid + 1, sample, hartid, "critical section ownership corrupted");
    inside_critical = 0;

    spin_unlock(&mutex_word);
    busy_delay(4 + hartid);
  }

  _barrier();

  if (hartid == 0) {
    intptr_t owner0 = 0;
    intptr_t owner1 = 0;
    intptr_t expected = LOCK_ITERS * NR_HARTS;

    expect(protected_counter == expected, sample, hartid, "protected counter mismatch");

    for (intptr_t i = 0; i < expected; i++) {
      expect(trace_owner[i] == 0 || trace_owner[i] == 1, sample, hartid, "invalid trace owner");
      owner0 += (trace_owner[i] == 0);
      owner1 += (trace_owner[i] == 1);
    }

    expect(owner0 == LOCK_ITERS, sample, hartid, "hart 0 trace count mismatch");
    expect(owner1 == LOCK_ITERS, sample, hartid, "hart 1 trace count mismatch");
    atomic_printf("[%s] protected counter=%ld owner0=%ld owner1=%ld\n",
        sample, protected_counter, owner0, owner1);
  }

  finish_case(sample, hartid);
}

int main(const char *args) {
  int hartid = read_hartid();
  enum sample_id sample = parse_sample(args);

  switch (sample) {
    case SAMPLE_SPLIT:
    case SAMPLE_MAILBOX:
    case SAMPLE_ATOMIC:
    case SAMPLE_LOCK:
      setup_dual_harts(hartid);
      break;
    case SAMPLE_HELP:
      if (hartid == 0) {
        print_usage();
      }
      return 0;
    case SAMPLE_INVALID:
    default:
      if (hartid == 0) {
        print_usage();
        printf("Unknown sample: %s\n", args == NULL ? "(null)" : args);
      }
      return 1;
  }

  switch (sample) {
    case SAMPLE_SPLIT:
      run_split_sample(hartid);
      break;
    case SAMPLE_MAILBOX:
      run_mailbox_sample(hartid);
      break;
    case SAMPLE_ATOMIC:
      run_atomic_sample(hartid);
      break;
    case SAMPLE_LOCK:
      run_lock_sample(hartid);
      break;
    case SAMPLE_HELP:
    case SAMPLE_INVALID:
    default:
      return 0;
  }

  return 0;
}
