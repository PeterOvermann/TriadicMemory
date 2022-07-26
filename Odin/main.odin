package main

import "core:fmt"
import "core:math/rand"
import "core:intrinsics"

import tri "triadic"

main :: proc() {
    n: int = 1000
    p: int = 10

    rng: rand.Rand = rand.create(u64(intrinsics.read_cycle_counter()))

    ttm := tri.temporal_memory_new(n, p)
    defer tri.temporal_memory_free(ttm)

    inputs: [20]tri.SDR

    fmt.println("Seq:")

    for t := 0; t < len(inputs); t += 1 {
        inputs[t] = tri.sdr_new_random(n, p, ttm.randomize_buffer, &rng)

        tri.sdr_print(inputs[t])
    }

    defer for t := 0; t < len(inputs); t += 1 {
        tri.sdr_free(inputs[t])
    }

    fmt.println("Training...")

    for it := 0; it < 20; it += 1 {
        for t := 0; t < len(inputs); t += 1 {
            tri.temporal_memory_step(ttm, inputs[t], &rng, true)
        }
    }

    fmt.println("Recall:")

    input := tri.sdr_new(n, p)
    defer tri.sdr_free(input)

    for t := 0; t < 20; t += 1 {
        tri.sdr_print(ttm.pred)

        tri.sdr_copy(&input, ttm.pred)
        tri.temporal_memory_step(ttm, input, &rng, false)
    }
}
