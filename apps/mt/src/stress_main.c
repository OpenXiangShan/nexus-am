#include <mt_common.h>

#define QUEUE_DEPTH 32
#define QUEUE_MASK (QUEUE_DEPTH - 1)
#define QUEUE_WINDOW 24
#define QUEUE_MESSAGES 8192

enum sample_id {
  SAMPLE_HELP = 0,
  SAMPLE_QUEUE,
  SAMPLE_INVALID,
};

typedef struct {
  uintptr_t seq;
  uintptr_t payload;
  uintptr_t token;
} queue_msg_t;

typedef struct {
  volatile uintptr_t head;
  volatile uintptr_t tail;
  queue_msg_t slots[QUEUE_DEPTH];
} spsc_queue_t;

static spsc_queue_t request_queue = {0};
static spsc_queue_t response_queue = {0};
static volatile uintptr_t queue_request_sum = 0;
static volatile uintptr_t queue_response_sum = 0;
static volatile uintptr_t queue_consume_sum = 0;
static volatile uintptr_t queue_verify_sum = 0;

static enum sample_id parse_sample(const char *args) {
  if (args == NULL || args[0] == '\0' || strcmp(args, "h") == 0 || strcmp(args, "help") == 0) {
    return SAMPLE_HELP;
  }
  if (strcmp(args, "5") == 0 || strcmp(args, "q") == 0 || strcmp(args, "queue") == 0) {
    return SAMPLE_QUEUE;
  }
  return SAMPLE_INVALID;
}

static void print_usage(void) {
  printf("Stress multi-hart samples:\n");
  printf("  build: make stress ARCH=riscv64-xs-dual STRESS_MAINARGS=<sample>\n");
  printf("  5 | queue : stress bidirectional SPSC message queues with fences\n");
}

static uintptr_t queue_payload_for(uintptr_t seq) {
  return ((seq + 1) * 0x45d9f3bULL) ^ (seq << 7) ^ 0x1234abcddcba4321ULL;
}

static uintptr_t queue_request_token(uintptr_t seq, uintptr_t payload) {
  return payload ^ (seq * 0x9e3779b97f4a7c15ULL) ^ 0x55aa55aa55aa55aaULL;
}

static uintptr_t queue_response_payload(uintptr_t seq, uintptr_t payload) {
  return (payload ^ 0xa5a5a5a5a5a5a5a5ULL) + (seq * 17);
}

static uintptr_t queue_response_token(uintptr_t seq, uintptr_t payload) {
  return payload ^ (seq * 0xc2b2ae3d27d4eb4fULL) ^ 0xaa55aa55aa55aa55ULL;
}

static void queue_push(spsc_queue_t *queue, queue_msg_t msg) {
  while ((queue->tail - queue->head) >= QUEUE_DEPTH) {
    cpu_relax();
  }

  uintptr_t tail = queue->tail;
  queue->slots[tail & QUEUE_MASK] = msg;
  fence_rw();
  queue->tail = tail + 1;
}

static queue_msg_t queue_pop(spsc_queue_t *queue) {
  while (queue->head == queue->tail) {
    cpu_relax();
  }

  fence_rw();
  uintptr_t head = queue->head;
  queue_msg_t msg = queue->slots[head & QUEUE_MASK];
  fence_rw();
  queue->head = head + 1;
  return msg;
}

static void run_queue_sample(int hartid) {
  const char *sample = "queue";

  if (hartid == 0) {
    uintptr_t sent = 0;
    uintptr_t received = 0;

    while (received < QUEUE_MESSAGES) {
      while (sent < QUEUE_MESSAGES && (sent - received) < QUEUE_WINDOW) {
        uintptr_t payload = queue_payload_for(sent);
        queue_msg_t request = {
          .seq = sent,
          .payload = payload,
          .token = queue_request_token(sent, payload),
        };
        queue_push(&request_queue, request);
        queue_request_sum += payload;
        sent++;
      }

      queue_msg_t response = queue_pop(&response_queue);
      uintptr_t expected_request = queue_payload_for(response.seq);
      uintptr_t expected_payload = queue_response_payload(response.seq, expected_request);

      expect(response.seq == received, sample, hartid, "response sequence mismatch");
      expect(response.payload == expected_payload, sample, hartid, "response payload mismatch");
      expect(response.token == queue_response_token(response.seq, response.payload),
          sample, hartid, "response token mismatch");
      queue_verify_sum += response.payload;
      received++;
    }
  } else if (hartid == 1) {
    for (uintptr_t received = 0; received < QUEUE_MESSAGES; received++) {
      queue_msg_t request = queue_pop(&request_queue);
      uintptr_t expected_payload = queue_payload_for(received);

      expect(request.seq == received, sample, hartid, "request sequence mismatch");
      expect(request.payload == expected_payload, sample, hartid, "request payload mismatch");
      expect(request.token == queue_request_token(request.seq, request.payload),
          sample, hartid, "request token mismatch");
      queue_consume_sum += request.payload;

      uintptr_t response_payload = queue_response_payload(request.seq, request.payload);
      queue_msg_t response = {
        .seq = request.seq,
        .payload = response_payload,
        .token = queue_response_token(request.seq, response_payload),
      };
      queue_push(&response_queue, response);
      queue_response_sum += response.payload;
    }
  } else {
    fail_now(sample, hartid, "unexpected hart");
  }

  _barrier();

  if (hartid == 0) {
    expect(queue_request_sum == queue_consume_sum, sample, hartid, "request sums differ");
    expect(queue_response_sum == queue_verify_sum, sample, hartid, "response sums differ");
    expect(request_queue.head == QUEUE_MESSAGES && request_queue.tail == QUEUE_MESSAGES,
        sample, hartid, "request queue did not drain");
    expect(response_queue.head == QUEUE_MESSAGES && response_queue.tail == QUEUE_MESSAGES,
        sample, hartid, "response queue did not drain");
    atomic_printf("[%s] messages=%d req_sum=0x%lx resp_sum=0x%lx depth=%d window=%d\n",
        sample, QUEUE_MESSAGES, queue_request_sum, queue_response_sum, QUEUE_DEPTH, QUEUE_WINDOW);
  }

  finish_case(sample, hartid);
}

int main(const char *args) {
  int hartid = read_hartid();
  enum sample_id sample = parse_sample(args);

  switch (sample) {
    case SAMPLE_QUEUE:
      setup_dual_harts(hartid);
      run_queue_sample(hartid);
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

  return 0;
}
