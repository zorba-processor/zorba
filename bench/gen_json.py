#!/usr/bin/env python3
"""Deterministic generator of the JSONiq benchmark dataset (customers + orders).

Usage: gen_json.py OUTPUT TARGET_MB

The output is one JSON object {"customers": [...], "orders": [...]} with one
record per line. The same arguments always produce byte-identical output
(own LCG, no dependency on the random module's version-specific behavior).
Bump GENERATOR_VERSION when the output changes: the CI data cache is keyed on
the hash of this file.
"""
import sys

GENERATOR_VERSION = 1

COUNTRIES = ["US", "DE", "FR", "MX", "BR", "JP", "IN", "CN", "GB", "ES",
             "IT", "CA", "AU", "NL", "SE", "PL", "AR", "ZA", "KR", "TR"]
SEGMENTS = ["consumer", "corporate", "small-business", "public"]
STATUSES = ["new", "paid", "shipped", "delivered", "cancelled", "returned"]
SKU_COUNT = 5000


class Lcg:
    def __init__(self, seed):
        self.s = seed & 0xFFFFFFFFFFFFFFFF

    def next(self):
        self.s = (self.s * 6364136223846793005 + 1442695040888963407) & 0xFFFFFFFFFFFFFFFF
        return self.s >> 33

    def below(self, n):
        return self.next() % n


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    path, target_mb = sys.argv[1], float(sys.argv[2])
    target = int(target_mb * 1024 * 1024)
    rnd = Lcg(20240229)
    n_customers = max(10, int(target_mb * 400))

    written = 0
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        def emit(s):
            nonlocal written
            f.write(s)
            written += len(s)

        emit('{"customers": [\n')
        for i in range(n_customers):
            emit('%s{"id": %d, "name": "Customer %d", "country": "%s", '
                 '"segment": "%s", "age": %d}\n'
                 % ("," if i else "", i, i, COUNTRIES[rnd.below(len(COUNTRIES))],
                    SEGMENTS[rnd.below(len(SEGMENTS))], 18 + rnd.below(70)))
        emit('], "orders": [\n')

        oid = 0
        while written < target:
            items = []
            total = 0.0
            for _ in range(1 + rnd.below(6)):
                qty = 1 + rnd.below(9)
                price = (100 + rnd.below(19900)) / 100.0
                total += qty * price
                items.append('{"sku": "SKU-%04d", "qty": %d, "price": %.2f}'
                             % (rnd.below(SKU_COUNT), qty, price))
            emit('%s{"id": %d, "customer": %d, "date": "20%02d-%02d-%02d", '
                 '"status": "%s", "total": %.2f, "items": [%s]}\n'
                 % ("," if oid else "", oid, rnd.below(n_customers),
                    10 + rnd.below(15), 1 + rnd.below(12), 1 + rnd.below(28),
                    STATUSES[rnd.below(len(STATUSES))], total, ", ".join(items)))
            oid += 1
        emit(']}\n')


if __name__ == "__main__":
    main()
