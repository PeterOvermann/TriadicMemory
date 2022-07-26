package triadic

import "core:math"
import "core:math/rand"
import "core:slice"
import "core:fmt"

SDR :: struct {
    p: int, // Number of bits used

    indices: []int, // Index store, size n
}

sdr_new :: proc(n: int, p: int = 0) -> SDR {
    // Empty SDR
    sdr := SDR{p, make([]int, n)}

    return sdr
}

sdr_new_random :: proc(n: int, p: int, randomize_buffer: []int, rng: ^rand.Rand) -> SDR {
    sdr := sdr_new(n, p)

    sdr_randomize(sdr, randomize_buffer, rng)

    return sdr
}

randomize_buffer_init :: proc(buffer: []int) {
    for _, i in buffer do buffer[i] = i
}

sdr_randomize :: proc(sdr: SDR, randomize_buffer: []int, rng: ^rand.Rand) {
    n: int = len(sdr.indices)

    assert(len(randomize_buffer) == n)

    for i := 0; i < sdr.p; i += 1 {
        r := int(rand.uint32(rng) % u32(n))

        sdr.indices[i] = randomize_buffer[r]

        // Swap
        tmp := randomize_buffer[n - 1]
        randomize_buffer[n - 1] = randomize_buffer[r]
        randomize_buffer[r] = tmp

        n -= 1
    }

    slice.sort(sdr.indices[:sdr.p])
}

sdr_free :: proc(sdr: SDR) {
    delete(sdr.indices)
}

sdr_copy :: proc(dst: ^SDR, src: SDR) {
    assert(len(dst.indices) == len(src.indices))

    dst.p = src.p

    for i := 0; i < dst.p; i += 1 do dst.indices[i] = src.indices[i]
}

sdr_or :: proc(dst: ^SDR, left: SDR, right: SDR) {
    assert(len(dst.indices) == len(left.indices) && len(dst.indices) == len(right.indices))

    // Zip together left and right SDRs into dst
    dst.p = 0

    l: int = 0
    r: int = 0

    for l < left.p || r < right.p {
        if l == left.p {
            for r < right.p {
                dst.indices[dst.p] = right.indices[r]
                dst.p += 1
                r += 1
            }
        }
        else if r == right.p {
            for l < left.p {
                dst.indices[dst.p] = left.indices[l]
                dst.p += 1
                l += 1
            }
        }
        else if left.indices[l] < right.indices[r] {
            dst.indices[dst.p] = left.indices[l]
            dst.p += 1
            l += 1
        }
        else if left.indices[l] > right.indices[r] {
            dst.indices[dst.p] = right.indices[r]
            dst.p += 1
            r += 1
        }
        else { // Matching indices
            dst.indices[dst.p] = left.indices[l]
            dst.p += 1
            l += 1
            r += 1
        }
    }
}

sdr_equal :: proc(left: SDR, right: SDR) -> bool {
    assert(len(left.indices) == len(right.indices))

    if left.p != right.p do return false 

    for i := 0; i < left.p; i += 1 {
        if left.indices[i] != right.indices[i] do return false
    }

    return true
}

sdr_distance :: proc(left: SDR, right: SDR) -> int {
    assert(len(left.indices) == len(right.indices))

    l: int = 0
    r: int = 0

    dist: int = left.p + right.p

    for l < left.p && r < right.p {
        if left.indices[l] == right.indices[l] {
            dist -= 2
            l += 1
            r += 1
        }
        else if left.indices[l] < right.indices[r] do l += 1
        else do r += 1
    }

    return dist
}

sdr_overlap :: proc(left: SDR, right: SDR) -> int {
    assert(len(left.indices) == len(right.indices))

    l: int = 0
    r: int = 0

    overlap: int = 0

    for l < left.p && r < right.p {
        if left.indices[l] == right.indices[l] {
            overlap += 1
            l += 1
            r += 1
        }
        else if left.indices[l] < right.indices[r] do l += 1
        else do r += 1
    }

    return overlap
}

sdr_print :: proc(sdr: SDR) {
    fmt.print("{")

    for i := 0; i < sdr.p; i += 1 {
        fmt.print(sdr.indices[i]);

        if i < sdr.p - 1 do fmt.print(" ")
    }

    fmt.println("}")
}

sdr_inhibit :: proc(dst: ^SDR, sums: []int, p: int) {
    // Use sdr as a temporary buffer
    for v, i in sums do dst.indices[i] = v
    
    slice.sort(dst.indices)

    threshold := dst.indices[len(dst.indices) - p]

    if threshold == 0 {
        dst.p = 0
    }
    else {
        dst.p = p

        index: int = 0

        for i := 0; i < len(dst.indices); i += 1 {
            if sums[i] >= threshold {
                dst.indices[index] = i
                index += 1
            }
        }
    }
}
