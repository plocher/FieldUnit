// Capacity guards: an add call beyond a container's size is refused and
// leaves the plant as it was. Each guard must test the size of the array
// it protects.
//
//   clang++ -std=c++17 -I src tests/test_capacity_guards.cpp -o /tmp/test_capacity_guards && /tmp/test_capacity_guards

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

int main() {
    printf("=== Capacity guards ===\n\n");

    // ---------------------------------------------------------
    printf("[TEST 1] The crossover after the last slot is refused, by pointer and by name\n");
    {
        InterlockingPlant cp("CROSSOVERS");
        char name[MAX_APPLIANCE_NAME_LEN];
        for (int i = 0; i < MAX_APPLIANCES; ++i) {
            snprintf(name, sizeof(name), "%d", 2 * i + 1);
            assert(cp.addSwitch(name) != nullptr);
        }
        SignalControl* sig = cp.addSignalControl("2");
        assert(sig != nullptr);

        for (int i = 0; i < MAX_CROSSOVERS; ++i) {
            snprintf(name, sizeof(name), "X%d", i);
            assert(cp.addCrossover(name, cp.getSwitch(2 * i), cp.getSwitch(2 * i + 1)) != nullptr);
        }
        assert(cp.crossoverCount() == MAX_CROSSOVERS);

        assert(cp.addCrossover("XP", cp.getSwitch(0), cp.getSwitch(1)) == nullptr);
        assert(cp.addCrossover("XN", "1", "3") == nullptr);

        // The refused calls changed nothing that lies after the crossovers.
        assert(cp.crossoverCount() == MAX_CROSSOVERS);
        assert(cp.findSwitch("XP") == nullptr);
        assert(cp.findSwitch("XN") == nullptr);
        assert(cp.authorityCount() == 1);
        assert(cp.findSignalControl("2") == sig);
        assert(strcmp(sig->name(), "2") == 0);
    }
    printf("   PASS\n\n");

    printf("All capacity guard tests passed.\n");
    return 0;
}
