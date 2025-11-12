Author: ChatGPT

You want to build TL2 in C++ “C-style,” solo, and still not hate yourself in three months. Charming.
Here’s a battle plan that keeps the codebase small, testable, and fast without sprinkling trendy template
glitter everywhere.

0) North-star goals

- Correctness property: opacity (no zombies), mixed malloc/free compatibility via per-stripe (PS) locks
plus quiescent reclamation. - Performance: read-only fast path; commit-time locking; minimal cache
traffic on the global version clock; low false sharing. - Maintainability: clear module seams, zero
exceptions, zero RTTI, boring CMake, ruthless logging.

1) Repo layout (one engineer, zero drama) /tl2/ build/ cmake/ include/tl2/ src/core/ bench/ tests/ docs/

2) Public surface (C-style, future-proof) - Opaque handle API. No templates, no exceptions, no STL
types in headers. - Functions: tx_begin(), tx_read(ptr), tx_write(ptr,val), tx_commit(), tx_abort(reason). -
Thread-local txn context. - Config via config.h + env vars. - Integer status codes, no throw.

3) Modern C++ allowed Allowed: - std::atomic with explicit memory orders. - alignas, thread_local,
std::chrono, std::thread (bench only). Avoid: - Exceptions, RTTI, shared_ptr, new/delete per op,
coroutines.

4) Core design decisions 4.1 Versioned write-lock layout: 1 bit lock, 63-bit version. 4.2 PS vs PO: ship
PS first. 4.3 GVC: atomic with fetch_add. 4.4 Read/write sets: fixed-capacity ring buffers, Bloom filter.
4.5 Validation: post-validate on read, revalidate on commit. 4.6 Contention: bounded spin + backoff. 4.
Reclamation: quiesce-before-free with epochs. 4.8 Memory orders: start seq_cst, add “aggressive”
profile later.

5) Observability - Per-thread stats: commits, aborts, read/write-set sizes. - JSON snapshots when
TX_STATS=1.

6) Correctness tests - Deterministic oracle comparison. - Delay injection and allocator churn. - Loop
safety test for opacity. - TSAN/ASAN builds for tests only.

7) Benchmarks Micro: - RB-tree, hash map, skip-list. Macro: - YCSB KV workload A/B/F, Zipfian
distributions, allocator churn. Metrics: throughput, latency, abort rate, scaling curves.

8) Performance methodology - Pin threads, discard warmup, median + MAD results. - Sweep stripe and
Bloom parameters, record heatmaps.

9) Config matrix - Stripe count, Bloom bits, backoff, validation, memory orders, epoch batch size.

10) Roadmap M1: Skeleton + Mutex baseline. M2: Read-only TL2. M3: Full TL2 write path. M4:
Quiescent reclamation. M5: Bench suite + plots. M6: Aggressive profile testing.

11) Style/build rules - -fno-exceptions -fno-rtti, Werror. - No STL in core headers.

12) Don’t forget - Cacheline align everything. - Stable abort reason enums. - Document every memory
order.

Stick to this skeleton: build → validate → benchmark → optimize. Clean, fast, empirical TL2.


