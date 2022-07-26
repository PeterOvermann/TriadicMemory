package triadic

import "core:math"
import "core:math/rand"

Temporal_Memory :: struct {
    m1, m2: Triadic_Memory,

    x, y, c, u, v, pred: SDR,

    randomize_buffer: []int,
}

temporal_memory_new :: proc(n: int, p: int) -> ^Temporal_Memory {
    ttm := new(Temporal_Memory)

    using ttm

    m1 = triadic_memory_new(n, p)
    m2 = triadic_memory_new(n, p)
    x = sdr_new(n, p)
    y = sdr_new(n, p)
    c = sdr_new(n, p)
    u = sdr_new(n, p)
    v = sdr_new(n, p)
    pred = sdr_new(n, p)
    randomize_buffer = make([]int, n)
    randomize_buffer_init(ttm.randomize_buffer)

    return ttm
}

temporal_memory_free :: proc(ttm: ^Temporal_Memory) {
    using ttm

    triadic_memory_free(m1)
    triadic_memory_free(m2)
    sdr_free(x)
    sdr_free(y)
    sdr_free(c)
    sdr_free(u)
    sdr_free(v)
    sdr_free(pred)
    delete(randomize_buffer)

    free(ttm)
}

temporal_memory_step :: proc(ttm: ^Temporal_Memory, input: SDR, rng: ^rand.Rand, learn_enabled: bool = true) -> SDR {
    using ttm

    if input.p == 0 {
        // Flush
        y.p = 0
        c.p = 0
        u.p = 0
        v.p = 0
        pred.p = 0

        return pred
    }

    sdr_or(&x, y, c)
    
    sdr_copy(&y, input)

    if !sdr_equal(pred, y) && learn_enabled do triadic_memory_add(m2, u, v, y)

    triadic_memory_read_z(m1, x, y, &c) // Recall c
    triadic_memory_read_x(m1, &u, y, c) // Recall u

    if sdr_overlap(x, u) < m1.p {
        c.p = m1.p // Resize for randomize

        sdr_randomize(c, randomize_buffer, rng)

        if learn_enabled do triadic_memory_add(m1, x, y, c)
    }

    sdr_copy(&u, x)
    sdr_copy(&v, y)

    triadic_memory_read_z(m2, u, v, &pred)

    return pred
}
