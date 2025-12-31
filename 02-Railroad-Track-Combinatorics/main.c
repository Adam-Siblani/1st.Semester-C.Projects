#include <stdio.h>

/**
 * Calculates the Greatest Common Divisor.
 * Useful for future optimizations involving Linear Diophantine Equations.
 */
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int main()
{
    long long a, b, target;
    char mode;

    /* Initial Input: Track lengths must be distinct and positive */
    printf("Track length:\n");
    if (scanf("%lld %lld", &a, &b) != 2) {
        printf("Invalid input.\n");
        return 0;
    }
    if (a <= 0 || b <= 0 || a == b) {
        printf("Invalid input.\n");
        return 0;
    }

    /* Distance Input: Requires a mode (+ or -) and a non-negative distance */
    printf("Distance:\n");
    if (scanf(" %c %lld", &mode, &target) != 2 || (mode != '+' && mode != '-') || target < 0) {
        printf("Invalid input.\n");
        return 0;
    }

    long long count = 0;

    /* Edge Case: If target is 0, there is exactly one way (0 of each track) */
    if (target == 0) {
        if (mode == '+')
            printf("= %lld * 0 + %lld * 0\n", a, b);
        printf("Total variants: 1\n");
        return 0;
    }

    /* 
     * Search Algorithm:
     * Iterates through possible counts of track 'a'. 
     * Checks if the remaining distance is perfectly divisible by track 'b'.
     */
    if (mode == '+') {
        for (long long x = 0; x <= target / a; x++) {
            long long rem = target - a * x;
            if (rem % b == 0) {
                long long y = rem / b;
                printf("= %lld * %lld + %lld * %lld\n", a, x, b, y);
                count++;
            }
        }
    }
    else {
        /* Optimized search for counting only (Bonus test compliant logic) */
        for (long long x = 0; x <= target / a; x++) {
            long long rem = target - a * x;
            if (rem % b == 0) {
                long long y = rem / b;
                if (y >= 0) count++;
            }
        }
    }

    /* Output Results */
    if (count == 0)
        printf("No solution.\n");
    else
        printf("Total variants: %lld\n", count);

    return 0;
}
