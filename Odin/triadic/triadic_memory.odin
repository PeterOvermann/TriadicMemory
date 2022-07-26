package triadic

import "core:math"
import "core:math/rand"

Triadic_Memory :: struct {
    n, p: int,

    mem: []u8,

    // Temporaries
    sums: []int,
}

triadic_memory_new :: proc(n: int, p: int) -> Triadic_Memory {
    tm := Triadic_Memory{n, p, make([]u8, n * n * n), make([]int, n)}

    return tm
}

triadic_memory_free :: proc(tm: Triadic_Memory) {
    delete(tm.mem)
    delete(tm.sums)
}

triadic_memory_add :: proc(tm: Triadic_Memory, x: SDR, y: SDR, z: SDR) {
    for i := 0; i < x.p; i += 1 {
        for j := 0; j < y.p; j += 1 {
            for k := 0; k < z.p; k += 1 do tm.mem[z.indices[k] + tm.n * (y.indices[j] + tm.n * x.indices[i])] += 1
        }
    }
}

triadic_memory_remove :: proc(tm: Triadic_Memory, x: SDR, y: SDR, z: SDR) {
    for i := 0; i < x.p; i += 1 {
        for j := 0; j < y.p; j += 1 {
            for k := 0; k < z.p; k += 1 {
                index := z.indices[k] + tm.n * (y.indices[j] + tm.n * x.indices[i])

                tm.mem[index] = max(0, tm.mem[index] - 1)
            }
        }
    }
}

triadic_memory_read_x :: proc(tm: Triadic_Memory, x: ^SDR, y: SDR, z: SDR) {
    for i := 0; i < tm.n; i += 1 {
        sum: int = 0

        for j := 0; j < y.p; j += 1 {
            for k := 0; k < z.p; k += 1 do sum += int(tm.mem[z.indices[k] + tm.n * (y.indices[j] + tm.n * i)])
        }

        tm.sums[i] = sum
    }

    sdr_inhibit(x, tm.sums, tm.p)
}

triadic_memory_read_y :: proc(tm: Triadic_Memory, x: SDR, y: ^SDR, z: SDR) {
    for j := 0; j < tm.n; j += 1 {
        sum: int = 0

        for i := 0; i < x.p; i += 1 {
            for k := 0; k < z.p; k += 1 do sum += int(tm.mem[z.indices[k] + tm.n * (j + tm.n * x.indices[i])])
        }

        tm.sums[j] = sum
    }

    sdr_inhibit(y, tm.sums, tm.p)
}

triadic_memory_read_z :: proc(tm: Triadic_Memory, x: SDR, y: SDR, z: ^SDR) {
    for k := 0; k < tm.n; k += 1 {
        sum: int = 0

        for i := 0; i < x.p; i += 1 {
            for j := 0; j < y.p; j += 1 do sum += int(tm.mem[k + tm.n * (y.indices[j] + tm.n * x.indices[i])])
        }

        tm.sums[k] = sum
    }

    sdr_inhibit(z, tm.sums, tm.p)
}
