// The train order face (FieldUnit ADR 0010): a ranked list of inputs, one
// indication out, no state between scans, and no part in the interlocking.
//
//   clang++ -std=c++17 -I src tests/test_train_order_face.cpp -o /tmp/test_train_order_face && /tmp/test_train_order_face

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

// The words the SPCoast model gives a face: STOP shows Stop, SLOW shows
// Approach, and nothing asserted shows Clear.
enum : uint8_t { STOP_INPUT = 0, SLOW_INPUT = 1 };

static void declareSpcoast(TrainOrderFace& face) {
    face.input(Indication::STOP).input(Indication::APPROACH);
}

int main() {
    printf("=== Train order face ===\n\n");

    // ---------------------------------------------------------
    printf("[TEST 1] D6: with no input asserted the face shows its rest indication\n");
    {
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);
        assert(face.inputCount() == 2);

        // A lever that was never sampled is a lever with nothing asserted.
        assert(face.evaluate() == Indication::CLEAR);
        assert(mast.currentIndication() == Indication::CLEAR);
        assert(mast.head1() == Aspect::GREEN);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 2] D1, D3: each input shows its word on the one-head mast\n");
    {
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);

        face.set(STOP_INPUT, true);
        assert(face.evaluate() == Indication::STOP);
        assert(mast.head1() == Aspect::RED);

        face.set(STOP_INPUT, false);
        face.set(SLOW_INPUT, true);
        assert(face.evaluate() == Indication::APPROACH);
        assert(mast.head1() == Aspect::YELLOW);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 3] D1: the higher rank wins, whichever input was asserted first in time\n");
    {
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);

        // SLOW first, then STOP.
        face.set(SLOW_INPUT, true);
        assert(face.evaluate() == Indication::APPROACH);
        face.set(STOP_INPUT, true);
        assert(face.evaluate() == Indication::STOP);

        // STOP first, then SLOW: the same result.
        face.set(STOP_INPUT, false);
        face.set(SLOW_INPUT, false);
        face.set(STOP_INPUT, true);
        assert(face.evaluate() == Indication::STOP);
        face.set(SLOW_INPUT, true);
        assert(face.evaluate() == Indication::STOP);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 4] D1: no state between scans; the result follows the present inputs\n");
    {
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);

        face.set(STOP_INPUT, true);
        assert(face.evaluate() == Indication::STOP);

        // The lever leaves STOP before it reaches SLOW: Clear for that moment.
        face.set(STOP_INPUT, false);
        assert(face.evaluate() == Indication::CLEAR);
        face.set(SLOW_INPUT, true);
        assert(face.evaluate() == Indication::APPROACH);

        // Back to rest. Nothing of the Stop or the Approach stays.
        face.set(SLOW_INPUT, false);
        assert(face.evaluate() == Indication::CLEAR);
        assert(mast.head1() == Aspect::GREEN);

        // resolve() gives the same answer and changes nothing.
        face.set(STOP_INPUT, true);
        assert(face.resolve() == Indication::STOP);
        assert(mast.currentIndication() == Indication::CLEAR);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 5] D2: the words and the rank are data given at setup\n");
    {
        // The other rank: the input declared first wins.
        SignalMast mastA("A", MastType::ONE_HEAD);
        TrainOrderFace slowFirst(&mastA, Indication::CLEAR);
        slowFirst.input(Indication::APPROACH).input(Indication::STOP);
        slowFirst.set(0, true);
        slowFirst.set(1, true);
        assert(slowFirst.evaluate() == Indication::APPROACH);

        // Another rest word and another set of words.
        SignalMast mastB("B", MastType::ONE_HEAD);
        TrainOrderFace other(&mastB, Indication::STOP);
        other.input(Indication::RESTRICTING);
        assert(other.rest() == Indication::STOP);
        assert(other.evaluate() == Indication::STOP);
        other.set(0, true);
        assert(other.evaluate() == Indication::RESTRICTING);
        assert(other.inputIndication(0) == Indication::RESTRICTING);

        // A face with a discrete CLEAR input (the owner, 2026-11-12): no
        // input is Clear, SLOW with CLEAR is Slow, STOP with anything is Stop.
        enum : uint8_t { STOP_IN = 0, SLOW_IN = 1, CLEAR_IN = 2 };
        SignalMast mastC("C", MastType::ONE_HEAD);
        TrainOrderFace three(&mastC, Indication::CLEAR);
        three.input(Indication::STOP).input(Indication::APPROACH).input(Indication::CLEAR);
        assert(three.inputCount() == 3);
        assert(three.evaluate() == Indication::CLEAR);        // no input
        three.set(CLEAR_IN, true);
        assert(three.evaluate() == Indication::CLEAR);        // CLEAR alone
        three.set(SLOW_IN, true);
        assert(three.evaluate() == Indication::APPROACH);     // SLOW and CLEAR
        three.set(STOP_IN, true);
        assert(three.evaluate() == Indication::STOP);         // STOP and both others
        three.set(SLOW_IN, false);
        assert(three.evaluate() == Indication::STOP);         // STOP and CLEAR
        three.set(CLEAR_IN, false);
        three.set(SLOW_IN, true);
        assert(three.evaluate() == Indication::STOP);         // STOP and SLOW
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 6] A fixed number of inputs: a call beyond it adds nothing\n");
    {
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        for (uint8_t i = 0; i < TrainOrderFace::MAX_INPUTS; ++i) {
            face.input(Indication::APPROACH);
        }
        face.input(Indication::STOP);
        assert(face.inputCount() == TrainOrderFace::MAX_INPUTS);

        // A rank that was not declared is ignored and reads not asserted.
        face.set(TrainOrderFace::MAX_INPUTS, true);
        face.set(200, true);
        assert(!face.asserted(TrainOrderFace::MAX_INPUTS));
        assert(face.evaluate() == Indication::CLEAR);

        // A face with no inputs and a face with no mast both give the rest word.
        TrainOrderFace empty(&mast, Indication::CLEAR);
        empty.set(0, true);
        assert(empty.evaluate() == Indication::CLEAR);
        TrainOrderFace noMast(nullptr, Indication::CLEAR);
        noMast.input(Indication::STOP);
        noMast.set(0, true);
        assert(noMast.evaluate() == Indication::STOP);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 7] D4: the face and its mast are outside the interlocking\n");
    {
        InterlockingPlant cp("Gilroy");
        cp.addTrackCircuit("777T");
        cp.addSignalMast("778NAB", MastType::TWO_HEAD);

        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);

        // The plant does not know the train order mast.
        assert(cp.mastCount() == 1);
        assert(cp.findSignalMast("Gilroy.E") == nullptr);

        // The plant's scan does not move the face, and the face does not
        // move the plant's mast.
        face.set(SLOW_INPUT, true);
        face.evaluate();
        cp.trackCircuit(0)->update(Occupancy::VACANT, Quality::GOOD, 1000);
        cp.tick(1000);
        assert(mast.currentIndication() == Indication::APPROACH);
        assert(cp.mast(0)->currentIndication() == Indication::STOP);

        // The train order mast is not in the indications the plant reports.
        IndicationVector ind;
        cp.exportIndicationVector(ind);
        AarTextCodec codec;
        codec.encodeIndications({ encodeTrack(cp.trackCircuit(0)) });
        char buf[128];
        size_t len = 0;
        assert(codec.encodeIndications(ind, buf, sizeof(buf), len));
        assert(strstr(buf, "Gilroy") == nullptr);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 8] D3: the semaphore driver that exists moves a train order arm\n");
    {
        MockIOBus io;
        SignalMast mast("Gilroy.E", MastType::ONE_HEAD);
        TrainOrderFace face(&mast, Indication::CLEAR);
        declareSpcoast(face);
        SemaphoreDriver arm(&mast);
        arm.addArm(0, 0, 0, 45, 90);

        face.evaluate();
        arm.drive(io);
        assert(io.readAngle(0, 0) == 90);

        face.set(SLOW_INPUT, true);
        face.evaluate();
        arm.drive(io);
        assert(io.readAngle(0, 0) == 45);

        face.set(STOP_INPUT, true);
        face.evaluate();
        arm.drive(io);
        assert(io.readAngle(0, 0) == 0);
    }
    printf("   PASS\n\n");

    printf("All train order face tests passed.\n");
    return 0;
}
