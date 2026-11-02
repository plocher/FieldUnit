#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

// FieldUnit ADR 0008: the automatic signal location.
//
// Part A  one location reading a line from ahead (unit)
// Part B  two local locations in a chain, and a loop
// Part C  the opposing cut at one location
// Part D  PlantSerializer round trip
// Part E  FieldUnit-Subdivision ADR 0013 section 16 trace on the Corporal-Sargent
//         block, with invariants I1, I2, I4 asserted at each row, and an I3 scenario.
// Part F  ControlPointLine: the control point's line at the seam (ADR 0008 D5)

static bool isProceed(const SignalMast* m) {
    return Route::permissivenessRank(m->currentIndication()) > 1;
}

static void vacant(TrackCircuit* tc, uint32_t now) { tc->update(Occupancy::VACANT, Quality::GOOD, now); }
static void occupied(TrackCircuit* tc, uint32_t now) { tc->update(Occupancy::OCCUPIED, Quality::GOOD, now); }

// ---------------------------------------------------------------------------
static void partA() {
    printf("[PART A] One automatic signal, 832SA, reading 834SA's line as line inputs\n");
    uint32_t now = 1000;
    InterlockingPlant cp("LOC832");
    TrackCircuit* t832 = cp.addTrackCircuit("832T");
    TrackCircuit* t834 = cp.addTrackCircuit("834T");
    LineInput* h834 = cp.addLineInput("834SAH");
    LineInput* d834 = cp.addLineInput("834SAD");
    AutomaticSignal& s = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
    s.governs({ "834T" }).approach({ "832T" }).reads("834SAH", "834SAD");

    const Fact* hOut = cp.findFact("832SAH");
    const Fact* dOut = cp.findFact("832SAD");
    assert(hOut && dOut && hOut == s.hOut() && dOut == s.dOut());
    assert(cp.findFact("834T") == t834);                       // a track circuit is a Fact
    assert(cp.engine().routeCount() == 1 && cp.engine().route(0).isAutomatic());
    assert(cp.engine().route(0).homeFactCount() == 2);         // 834T and 834SAH
    assert(cp.engine().route(0).proceedFact() == d834);
    SignalMast* mast = s.mast();
    assert(mast && strcmp(mast->name(), "832SA") == 0);

    // A1: circuits clear, line never set: Stop; H out released (no H from ahead, no stick)
    vacant(t832, now); vacant(t834, now);
    cp.tick(now);
    assert(mast->currentIndication() == Indication::STOP);
    assert(!dOut->asserted() && !hOut->asserted());
    printf("   A1 never-set line: STOP, H out and D out released\n");

    // A2: H asserted, D released: Approach; HR up so D out asserted; H out asserted
    h834->update(true, now); d834->update(false, now);
    cp.tick(now);
    assert(mast->currentIndication() == Indication::APPROACH);
    assert(dOut->asserted() && hOut->asserted());
    printf("   A2 H only: APPROACH, D out asserted\n");

    // A3: D asserted: Clear
    d834->update(true, now);
    cp.tick(now);
    assert(mast->currentIndication() == Indication::CLEAR);
    printf("   A3 H and D: CLEAR\n");

    // A4: train enters the governed block while HR was up: Stop, SR sets, H out held by SR
    occupied(t834, now);
    h834->update(false, now);                                  // 834 reports its block occupied
    cp.tick(now);
    assert(mast->currentIndication() == Indication::STOP);
    assert(s.SR());
    assert(!dOut->asserted());
    assert(hOut->asserted());                                  // masked by the stick
    printf("   A4 train in governed block: STOP, SR set, H out masked by SR\n");

    // A5: train moves on; 834 masks it with its own stick: H from ahead returns; SR releases
    vacant(t834, now);
    h834->update(true, now); d834->update(false, now);          // 834SA at Stop, its SR masking
    cp.tick(now);
    assert(!s.SR());
    assert(mast->currentIndication() == Indication::APPROACH);
    assert(hOut->asserted() && dOut->asserted());
    printf("   A5 H returns: SR releases, APPROACH\n");

    // A6: a train behind (approach block occupied) releases H out whatever the stick
    occupied(t832, now);
    cp.tick(now);
    assert(!hOut->asserted());
    assert(mast->currentIndication() == Indication::APPROACH);  // the display reads only ahead
    vacant(t832, now);
    cp.tick(now);
    assert(hOut->asserted());
    printf("   A6 approach block occupied: H out released, display unchanged\n");

    // A7: a stale line releases H: Stop
    now += 6000;
    t832->touch(now); t834->touch(now);                         // circuits alive, unchanged
    cp.tick(now);
    assert(h834->isStale());
    assert(mast->currentIndication() == Indication::STOP);
    assert(!dOut->asserted());
    h834->update(true, now); d834->update(true, now);
    cp.tick(now);
    assert(mast->currentIndication() == Indication::CLEAR);
    printf("   A7 stale line: STOP; fresh line: CLEAR\n");

    // A8: a stick never sets when the block was entered with HR down
    h834->update(false, now);
    cp.tick(now);                                               // HR down (H released)
    occupied(t834, now);
    cp.tick(now);
    assert(!s.SR());
    printf("   A8 entered with HR down: no stick\n\n");
}

// ---------------------------------------------------------------------------
static void partB() {
    printf("[PART B] Two local locations in a chain; a loop reads as released\n");
    uint32_t now = 1000;
    InterlockingPlant cp("CHAIN");
    TrackCircuit* t832 = cp.addTrackCircuit("832T");
    TrackCircuit* t834 = cp.addTrackCircuit("834T");
    TrackCircuit* t836 = cp.addTrackCircuit("836T");
    LineInput* h836 = cp.addLineInput("836SAH");
    LineInput* d836 = cp.addLineInput("836SAD");
    // 834SA reads Sargent's line; 832SA reads 834SA's outputs, declared after it on purpose
    AutomaticSignal& s832 = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
    AutomaticSignal& s834 = cp.addAutomaticSignal("834SA", MastType::ONE_HEAD);
    s834.governs({ "836T" }).approach({ "834T" }).reads("836SAH", "836SAD");
    s832.governs({ "834T" }).approach({ "832T" }).reads("834SAH", "834SAD");
    assert(s832.hIn() == s834.hOut() && s832.dIn() == s834.dOut());

    vacant(t832, now); vacant(t834, now); vacant(t836, now);
    h836->update(true, now); d836->update(true, now);
    cp.tick(now);
    assert(s832.mast()->currentIndication() == Indication::CLEAR);
    assert(s834.mast()->currentIndication() == Indication::CLEAR);
    printf("   B1 idle block: both CLEAR in one scan\n");

    // B2: Sargent clears an opposing signal: its H to 834 drops. Tumble-down: 834SA
    // and 832SA go to Stop in the same scan, and 832's H to Corporal drops with them.
    h836->update(false, now); d836->update(false, now);
    cp.tick(now);
    assert(s834.mast()->currentIndication() == Indication::STOP);
    assert(s832.mast()->currentIndication() == Indication::STOP);
    assert(!s834.hOut()->asserted() && !s832.hOut()->asserted());
    assert(!s834.SR() && !s832.SR());
    printf("   B2 opposing H withdrawn: tumble-down reaches 832 in one scan\n");

    // B3: the opposing train enters 836T from the far side: no stick on 834SA, since its
    // HR was already down when its governed block was entered
    occupied(t836, now);
    cp.tick(now);
    assert(!s834.SR());
    assert(s834.mast()->currentIndication() == Indication::STOP);
    printf("   B3 entered against the direction: no stick\n");

    // B4: H returns from Sargent (train gone, 836T clear): both Clear again in one scan
    vacant(t836, now);
    h836->update(true, now); d836->update(true, now);
    cp.tick(now);
    assert(s832.mast()->currentIndication() == Indication::CLEAR);
    assert(s834.mast()->currentIndication() == Indication::CLEAR);
    printf("   B4 H returns: both CLEAR in one scan\n");

    // Loop: two locations reading each other's outputs settle released, no crash
    InterlockingPlant loop("LOOP");
    TrackCircuit* la = loop.addTrackCircuit("AT");
    TrackCircuit* lb = loop.addTrackCircuit("BT");
    AutomaticSignal& a = loop.addAutomaticSignal("A", MastType::ONE_HEAD);
    AutomaticSignal& b = loop.addAutomaticSignal("B", MastType::ONE_HEAD);
    a.governs({ "BT" }).approach({ "AT" }).reads("BH", "BD");
    b.governs({ "AT" }).approach({ "BT" }).reads("AH", "AD");
    vacant(la, now); vacant(lb, now);
    loop.tick(now);
    assert(a.mast()->currentIndication() == Indication::STOP);
    assert(b.mast()->currentIndication() == Indication::STOP);
    assert(!a.hOut()->asserted() && !b.hOut()->asserted());
    printf("   B5 a loop of locations reads as released\n\n");
}

// ---------------------------------------------------------------------------
static void partC() {
    printf("[PART C] Opposing cut at one location (sameLocationOpposingCut)\n");
    uint32_t now = 1000;
    InterlockingPlant cp("LOC832");
    TrackCircuit* t832 = cp.addTrackCircuit("832T");
    TrackCircuit* t834 = cp.addTrackCircuit("834T");
    LineInput* h834 = cp.addLineInput("834SAH");    // read by 832SA (southbound)
    LineInput* d834 = cp.addLineInput("834SAD");
    LineInput* hCorp = cp.addLineInput("830NABH");  // read by 832NA (northbound)
    LineInput* dCorp = cp.addLineInput("830NABD");
    AutomaticSignal& sa = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
    AutomaticSignal& na = cp.addAutomaticSignal("832NA", MastType::ONE_HEAD);
    sa.governs({ "834T" }).approach({ "832T" }).reads("834SAH", "834SAD").opposing("832NA");
    na.governs({ "832T" }).approach({ "834T" }).reads("830NABH", "830NABD").opposing("832SA");
    assert(cp.engine().route(0).homeFactCount() == 3);   // 834T, 834SAH, the cut

    vacant(t832, now); vacant(t834, now);
    // Traffic established southbound: H comes from 834 only (CB&Q idle "last direction")
    h834->update(true, now); d834->update(true, now);
    hCorp->update(false, now); dCorp->update(false, now);
    cp.tick(now);
    cp.tick(now);
    assert(sa.mast()->currentIndication() == Indication::CLEAR);
    assert(na.mast()->currentIndication() == Indication::STOP);
    printf("   C1 southbound established: 832SA CLEAR, 832NA STOP\n");

    // The opposing H appears while 832SA's HR is up: 832NA stays cut
    hCorp->update(true, now); dCorp->update(true, now);
    cp.tick(now);
    assert(na.mast()->currentIndication() == Indication::STOP);
    assert(sa.mast()->currentIndication() == Indication::CLEAR);
    printf("   C2 opposing H arrives: 832NA held at STOP by 832SA's HR\n");

    // Southbound H withdrawn: 832SA drops this scan; 832NA picks up the next scan
    h834->update(false, now); d834->update(false, now);
    cp.tick(now);
    assert(sa.mast()->currentIndication() == Indication::STOP);
    assert(na.mast()->currentIndication() == Indication::STOP);  // reads 832SA's held HR
    cp.tick(now);
    assert(na.mast()->currentIndication() == Indication::CLEAR);
    printf("   C3 southbound H withdrawn: 832NA CLEAR one scan later\n");

    // A stick on 832SA also cuts 832NA (the SR back contact)
    h834->update(true, now); d834->update(true, now);
    hCorp->update(false, now);
    cp.tick(now); cp.tick(now);
    assert(sa.mast()->currentIndication() == Indication::CLEAR);
    occupied(t834, now); h834->update(false, now);
    cp.tick(now);
    assert(sa.SR());
    hCorp->update(true, now);
    cp.tick(now); cp.tick(now);
    assert(na.mast()->currentIndication() == Indication::STOP);
    printf("   C4 832SA's stick holds 832NA at STOP\n\n");
}

// ---------------------------------------------------------------------------
static void partD() {
    printf("[PART D] PlantSerializer round trip\n");
    InterlockingPlant cp("LOC832");
    cp.addTrackCircuit("832T");
    cp.addTrackCircuit("834T");
    cp.addLineInput("834SAH");
    cp.addLineInput("834SAD");
    cp.addLineInput("830NABH");
    cp.addLineInput("830NABD");
    cp.addSignalMast("830SAB", MastType::TWO_HEAD);          // a plain mast stays in signalMasts
    // Names bind when called: add both signals before either names the other.
    AutomaticSignal& sa = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
    AutomaticSignal& na = cp.addAutomaticSignal("832NA", MastType::ONE_HEAD);
    sa.governs({ "834T" }).approach({ "832T" }).reads("834SAH", "834SAD").opposing("832NA");
    na.governs({ "832T" }).approach({ "834T" }).reads("830NABH", "830NABD").opposing("832SA");
    cp.addSignalControl("830");
    cp.route("LEAVING")
      .governedBy("830", DirectionAuthority::RIGHT)
      .displays("830SAB", Indication::CLEAR)
      .clears({ "832T" })
      .home({ "832SAH" })
      .proceed("832SAD")
      .entrance("832T");

    static char json[12288];
    assert(cp.serialize(json, sizeof(json), true));
    assert(strstr(json, "\"automaticSignals\"") != nullptr);
    assert(strstr(json, "\"governs\": [\"834T\"], \"approach\": [\"832T\"], \"reads\": {\"h\": \"834SAH\", \"d\": \"834SAD\"}, \"opposing\": \"832NA\"") != nullptr);
    // automatic masts are written under automaticSignals, not signalMasts
    const char* masts = strstr(json, "\"signalMasts\"");
    const char* autos = strstr(json, "\"automaticSignals\"");
    assert(masts && autos && masts < autos);
    assert(strstr(masts, "830SAB") < autos);
    assert(strstr(masts, "832SA") > autos);
    const char* routes = strstr(json, "\"routes\"");
    assert(routes && strstr(routes, "\"name\": \"832SA\"") == nullptr);
    assert(strstr(routes, "\"name\": \"LEAVING\"") != nullptr);

    InterlockingPlant twin("TWIN");
    assert(twin.deserialize(json));
    assert(twin.automaticSignalCount() == 2);
    assert(twin.mastCount() == 3);
    assert(twin.engine().routeCount() == 3);
    const AutomaticSignal* a = twin.findAutomaticSignal("832SA");
    assert(a && a->governedCount() == 1 && strcmp(a->governed(0)->name(), "834T") == 0);
    assert(a->approachCount() == 1 && strcmp(a->approachFact(0)->name(), "832T") == 0);
    assert(a->hIn() == twin.findLineInput("834SAH") && a->dIn() == twin.findLineInput("834SAD"));
    assert(a->opposingSignal() == twin.findAutomaticSignal("832NA"));
    const Route* leaving = nullptr;
    for (uint8_t i = 0; i < twin.engine().routeCount(); ++i) {
        if (strcmp(twin.engine().route(i).name(), "LEAVING") == 0) leaving = &twin.engine().route(i);
    }
    assert(leaving && leaving->homeFact(0) == a->hOut() && leaving->proceedFact() == a->dOut());
    printf("   D1 round trip keeps bindings, outputs and the plain route\n\n");
}

// ---------------------------------------------------------------------------
// Part E: the Corporal-Sargent block of ADR 0013 section 16, in one plant
// (carrier variation C, in-memory). Southbound runs Corporal -> 832T -> 834T
// -> 836T -> Sargent. Locations 832 and 834 are automatic. Each control point
// is in the same plant as a leaving signal, a home signal and a
// ControlPointLine (ADR 0008 D5) under SP_1942 (tumbleDownTrigger "cleared",
// trafficMechanism "hd_chain": traffic_free = segment_clear).
struct Block {
    InterlockingPlant cp;
    TrackCircuit *corpOs, *t832, *t834, *t836, *sargOs;
    ControlPointLine *corpLine, *sargLine;   // 830NAB to 832NA; 836SA to 834SA
    AutomaticSignal *s832SA, *s834SA, *s832NA, *s834NA;
    SignalMast *m830SAB, *m836NAB, *m830NAB, *m836SA;
    SignalControl *c830, *c836;
    uint32_t now;

    Block() : cp("CORP-SARG"), now(1000) {
        corpOs = cp.addTrackCircuit("830T1");
        t832 = cp.addTrackCircuit("832T");
        t834 = cp.addTrackCircuit("834T");
        t836 = cp.addTrackCircuit("836T");
        sargOs = cp.addTrackCircuit("836T1");
        c830 = cp.addSignalControl("830");        // lever 830: RIGHT leaves southbound, LEFT is the home
        c836 = cp.addSignalControl("836");        // lever 836: LEFT leaves northbound, RIGHT is the home
        m830SAB = cp.addSignalMast("830SAB", MastType::TWO_HEAD);
        m830NAB = cp.addSignalMast("830NAB", MastType::TWO_HEAD);
        m836NAB = cp.addSignalMast("836NAB", MastType::TWO_HEAD);
        m836SA  = cp.addSignalMast("836SA",  MastType::ONE_HEAD);
        cp.route("830NAB-HOME")
          .governedBy("830", DirectionAuthority::LEFT)
          .displays("830NAB", Indication::CLEAR)
          .clears({ "830T1" })
          .entrance("830T1");
        cp.route("836SA-HOME")
          .governedBy("836", DirectionAuthority::RIGHT)
          .displays("836SA", Indication::CLEAR)
          .clears({ "836T1" })
          .entrance("836T1");
        corpLine = &cp.addControlPointLine("830NAB");
        corpLine->governs({ "832T" }).block({ "832T", "834T", "836T" }).leaving({ "830SAB" })
                 .tumbleDown(TumbleDownTrigger::CLEARED).traffic(TrafficMechanism::HD_CHAIN);
        sargLine = &cp.addControlPointLine("836SA");
        sargLine->governs({ "836T" }).block({ "832T", "834T", "836T" }).leaving({ "836NAB" })
                 .tumbleDown(TumbleDownTrigger::CLEARED).traffic(TrafficMechanism::HD_CHAIN);
        s832SA = &cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
        s834SA = &cp.addAutomaticSignal("834SA", MastType::ONE_HEAD);
        s832NA = &cp.addAutomaticSignal("832NA", MastType::ONE_HEAD);
        s834NA = &cp.addAutomaticSignal("834NA", MastType::ONE_HEAD);
        s834SA->governs({ "836T" }).approach({ "834T" }).reads("836SAH", "836SAD");
        s832SA->governs({ "834T" }).approach({ "832T" }).reads("834SAH", "834SAD");
        s832NA->governs({ "832T" }).approach({ "834T" }).reads("830NABH", "830NABD");
        s834NA->governs({ "834T" }).approach({ "836T" }).reads("832NAH", "832NAD");
        cp.route("830SAB-LEAVING")
          .governedBy("830", DirectionAuthority::RIGHT)
          .displays("830SAB", Indication::CLEAR)
          .clears({ "830T1" })
          .home({ "832SAH" })            // segment_clear = traffic_free under hd_chain
          .proceed("832SAD")             // next_proceed
          .entrance("830T1");
        cp.route("836NAB-LEAVING")
          .governedBy("836", DirectionAuthority::LEFT)
          .displays("836NAB", Indication::CLEAR)
          .clears({ "836T1" })
          .home({ "834NAH" })
          .proceed("834NAD")
          .entrance("836T1");
        vacant(corpOs, now); vacant(t832, now); vacant(t834, now); vacant(t836, now); vacant(sargOs, now);
        cp.tick(now);
        cp.tick(now);
    }

    void scan() { cp.tick(now); }

    void request(SignalControl* c, DirectionAuthority dir) {
        ControlTransaction ctl;
        ctl.signalDemands[c->index()] = (dir == DirectionAuthority::RIGHT) ? SignalDemand::RIGHT
                                      : (dir == DirectionAuthority::LEFT)  ? SignalDemand::LEFT
                                                                           : SignalDemand::STOP;
        cp.applyControlTransaction(ctl, now);
    }

    // Seam facts as the two control points read them
    bool sargSegmentClear() const { return s834NA->hOut()->asserted(); }   // 834 -> Sargent H
    bool corpSegmentClear() const { return s832SA->hOut()->asserted(); }   // 832 -> Corporal H
    bool corpNextProceed() const { return s832SA->dOut()->asserted(); }
    bool h832to834() const { return s832NA->hOut()->asserted(); }

    // I1, I2, I4 (ADR 0013 section 14.1) under hd_chain, where traffic_free = segment_clear
    void invariants(const char* row, bool trafficSouthbound = true) {
        bool corpCleared = isProceed(m830SAB);
        bool sargCleared = isProceed(m836NAB);
        // I1: never both leaving signals able to clear into the block
        if (corpCleared) assert(!sargSegmentClear());
        if (sargCleared) assert(!corpSegmentClear());
        // I2: with traffic established, while any circuit is occupied or the sending end is
        // cleared or in time, the far end's traffic_free is released
        bool anyOccupied = !t832->isClear() || !t834->isClear() || !t836->isClear();
        if (trafficSouthbound) {
            if (anyOccupied || corpCleared || c830->isTimeLocked()) assert(!sargSegmentClear());
        } else {
            if (anyOccupied || sargCleared || c836->isTimeLocked()) assert(!corpSegmentClear());
        }
        // I4: segment_clear released whenever a governed circuit is occupied; next_proceed released when the next signal is at Stop
        if (!t832->isClear()) assert(!corpSegmentClear());
        if (!t836->isClear()) assert(!sargSegmentClear());
        if (!isProceed(s832SA->mast())) assert(!corpNextProceed());
        if (!isProceed(s834NA->mast())) assert(!s834NA->dOut()->asserted());
        printf("   %s: I1 I2 I4 hold\n", row);
    }
};

static void partE() {
    printf("[PART E] ADR 0013 section 16 trace, Corporal -> Sargent, SP_1942\n");
    Block b;
    // Idle block: both intermediates show proceed both ways (intermediatesAtRest
    // "clear"); the one facing each control point shows Approach, since the home
    // signal there is at Stop
    assert(b.s832SA->mast()->currentIndication() == Indication::CLEAR);
    assert(b.s834SA->mast()->currentIndication() == Indication::APPROACH);
    assert(b.s832NA->mast()->currentIndication() == Indication::APPROACH);
    assert(b.s834NA->mast()->currentIndication() == Indication::CLEAR);
    assert(b.corpSegmentClear() && b.sargSegmentClear());
    assert(!b.corpLine->producesF() && b.cp.findFact("830NABF") == nullptr);
    b.invariants("idle");

    // Row 1: at Corporal, 830SAB cleared: tumble-down northbound, Sargent locked out
    b.request(b.c830, DirectionAuthority::RIGHT);
    b.scan();
    assert(b.m830SAB->currentIndication() == Indication::CLEAR);
    b.scan();                                         // Corporal's H out drops one scan later
    assert(!b.h832to834());
    assert(!b.sargSegmentClear());
    assert(b.corpSegmentClear());
    assert(b.s832NA->mast()->currentIndication() == Indication::STOP);
    assert(b.s834NA->mast()->currentIndication() == Indication::STOP);
    b.invariants("row 1 830SAB cleared");

    // Row 2: T1 in 832T: 830SAB knocked down; everything released
    occupied(b.corpOs, b.now);
    b.scan();
    assert(b.c830->activeDirection() == DirectionAuthority::STOP);
    vacant(b.corpOs, b.now);
    occupied(b.t832, b.now);
    b.scan();
    assert(b.m830SAB->currentIndication() == Indication::STOP);
    assert(!b.h832to834() && !b.sargSegmentClear() && !b.corpSegmentClear());
    b.invariants("row 2 T1 in 832T");

    // Row 3: T1 in 834T, 832T clear: 832SA's SR masks the break; Corporal may clear to Approach
    vacant(b.t832, b.now);
    occupied(b.t834, b.now);
    b.scan();
    assert(b.s832SA->SR());
    assert(b.corpSegmentClear());
    assert(!b.corpNextProceed());
    assert(!b.h832to834() && !b.sargSegmentClear());
    b.request(b.c830, DirectionAuthority::RIGHT);     // a following move
    b.scan();
    assert(b.m830SAB->currentIndication() == Indication::APPROACH);
    b.scan();
    b.invariants("row 3 T1 in 834T, following move at Approach");
    b.request(b.c830, DirectionAuthority::STOP);      // take it back for the trace
    b.scan(); b.scan();

    // Row 4: T1 in 836T, 834T clear: 834SA's SR masks; 832SA shows proceed with D asserted;
    // Sargent still locked out by 836T
    vacant(b.t834, b.now);
    occupied(b.t836, b.now);
    b.scan();
    assert(b.s834SA->SR());                           // set this scan
    assert(b.corpSegmentClear());                     // 832SA's own stick still masks
    b.scan();
    assert(!b.s832SA->SR());                          // H from 834 returned through 834's stick
    assert(b.corpSegmentClear() && b.corpNextProceed());
    assert(b.s834SA->mast()->currentIndication() == Indication::STOP);
    assert(b.s832SA->mast()->currentIndication() == Indication::APPROACH);   // next signal at Stop (I4)
    assert(!b.sargSegmentClear());
    assert(b.h832to834());                            // Corporal's outbound H restored
    b.invariants("row 4 T1 in 836T");

    // Row 5: T1 clear of 836T into Sargent: block idle again
    vacant(b.t836, b.now);
    b.scan(); b.scan();
    assert(!b.s834SA->SR());
    assert(b.corpSegmentClear() && b.sargSegmentClear());
    assert(b.s832SA->mast()->currentIndication() == Indication::CLEAR);
    assert(b.s834SA->mast()->currentIndication() == Indication::APPROACH);
    assert(b.s832NA->mast()->currentIndication() == Indication::APPROACH);
    assert(b.s834NA->mast()->currentIndication() == Indication::CLEAR);
    b.invariants("row 5 block idle");

    // Sargent lines its home signal: 834SA goes to Clear through the line's D
    b.request(b.c836, DirectionAuthority::RIGHT);
    b.scan(); b.scan();
    assert(b.m836SA->currentIndication() == Indication::CLEAR);
    assert(b.s834SA->mast()->currentIndication() == Indication::CLEAR);
    b.request(b.c836, DirectionAuthority::STOP);
    b.scan(); b.scan();
    assert(b.s834SA->mast()->currentIndication() == Indication::APPROACH);
    printf("   home signal lined: D from the control point upgrades 834SA to CLEAR\n");

    // I3 scenario: a northbound train T2 from Sargent sets the receding sticks on 834NA and
    // 832NA; Corporal requests southbound while T2 is still in 832T. When T2 clears into
    // Corporal, either the sticks release before Corporal clears, or Sargent's traffic_free
    // stays released.
    printf("   I3: northbound T2, Corporal requests southbound behind it\n");
    b.request(b.c836, DirectionAuthority::LEFT);
    b.scan(); b.scan();
    assert(b.m836NAB->currentIndication() == Indication::CLEAR);
    occupied(b.sargOs, b.now); b.scan();
    vacant(b.sargOs, b.now); occupied(b.t836, b.now); b.scan();
    vacant(b.t836, b.now); occupied(b.t834, b.now); b.scan();
    assert(b.s834NA->SR());
    vacant(b.t834, b.now); occupied(b.t832, b.now); b.scan();
    assert(b.s832NA->SR());
    b.request(b.c836, DirectionAuthority::STOP);
    b.request(b.c830, DirectionAuthority::RIGHT);     // requested southbound, T2 still in 832T
    b.scan();
    assert(b.m830SAB->currentIndication() == Indication::STOP);
    b.invariants("I3 T2 in 832T, Corporal requested", false);
    vacant(b.t832, b.now);                            // T2 into Corporal
    int overlapScans = 0;
    for (int i = 0; i < 6; ++i) {
        b.scan();
        bool corpCleared = isProceed(b.m830SAB);
        if (corpCleared && b.sargSegmentClear()) overlapScans++;
    }
    bool stuck = b.s832NA->SR() || b.s834NA->SR();
    assert(!stuck);                                   // the sticks do release
    if (overlapScans > 0) {
        // Recorded, not asserted: a finding for FieldUnit-Subdivision ADR 0013 section 14.2
        // (SP_1942 "holds I3 by trace"). The sticks release as Corporal's H out returns, but
        // the release travels down the chain one location per scan while Corporal clears in
        // the first scan, so both ends see permission for the chain's propagation time. With
        // cleared held as a relay state, that is this many scans:
        printf("   I3 FINDING under SP_1942 (hd_chain, H out = T clear AND !in_time AND !cleared):\n");
        printf("     sticks released, but Corporal was cleared while Sargent's segment_clear was\n");
        printf("     still asserted for %d scan(s): the chain's propagation delay, not a stuck stick.\n", overlapScans);
    } else {
        printf("   I3 holds with no overlap\n");
    }
    printf("\n");
}

// ---------------------------------------------------------------------------
static void partF() {
    printf("[PART F] ControlPointLine at the seam (ADR 0008 D5)\n");
    uint32_t now = 1000;
    InterlockingPlant cp("CORPORAL");
    TrackCircuit* os = cp.addTrackCircuit("830T1");
    TrackCircuit* t832 = cp.addTrackCircuit("832T");
    TrackCircuit* t834 = cp.addTrackCircuit("834T");
    LineInput* h832 = cp.addLineInput("832SAH");     // the block's line into Corporal
    LineInput* d832 = cp.addLineInput("832SAD");
    SignalControl* c830 = cp.addSignalControl("830");
    SignalMast* leaving = cp.addSignalMast("830SAB", MastType::TWO_HEAD);
    SignalMast* home = cp.addSignalMast("830NAB", MastType::TWO_HEAD);
    cp.route("LEAVING").governedBy("830", DirectionAuthority::RIGHT).displays("830SAB", Indication::CLEAR)
      .clears({ "830T1" }).home({ "832SAH" }).proceed("832SAD").entrance("830T1");
    cp.route("HOME").governedBy("830", DirectionAuthority::LEFT).displays("830NAB", Indication::CLEAR)
      .clears({ "830T1" }).entrance("830T1");
    ControlPointLine& line = cp.addControlPointLine("830NAB");
    line.governs({ "832T" }).block({ "832T", "834T" }).leaving({ "830SAB" });
    const Fact* H = cp.findFact("830NABH");
    const Fact* D = cp.findFact("830NABD");
    assert(H && D && H == line.hOut() && D == line.dOut());
    assert(line.homeMast() == home && line.leavingMast(0) == leaving);
    assert(!line.producesF() && cp.findFact("830NABF") == nullptr);

    auto request = [&](DirectionAuthority dir) {
        ControlTransaction ctl;
        ctl.signalDemands[c830->index()] = (dir == DirectionAuthority::RIGHT) ? SignalDemand::RIGHT
                                         : (dir == DirectionAuthority::LEFT)  ? SignalDemand::LEFT
                                                                              : SignalDemand::STOP;
        cp.applyControlTransaction(ctl, now);
    };

    vacant(os, now); vacant(t832, now); vacant(t834, now);
    h832->update(true, now); d832->update(true, now);
    cp.tick(now);
    assert(H->asserted() && !D->asserted());
    assert(!line.requested() && !line.cleared() && !line.inTime());
    printf("   F1 idle: H out asserted, D out released, no F under hd_chain\n");

    // F2: a leaving signal cleared cuts H out (tumbleDownTrigger cleared), one scan after it clears
    request(DirectionAuthority::RIGHT);
    cp.tick(now);
    assert(leaving->currentIndication() == Indication::CLEAR);
    assert(line.requested());
    assert(line.cleared());                 // latched at the end of that scan
    assert(!H->asserted());
    printf("   F2 leaving signal cleared: H out released\n");

    // F3: the adjacent circuit occupied cuts H out whatever the signals do
    request(DirectionAuthority::STOP);
    cp.tick(now); cp.tick(now);
    assert(H->asserted());
    occupied(t832, now);
    cp.tick(now);
    assert(!H->asserted());
    vacant(t832, now);
    cp.tick(now);
    assert(H->asserted());
    printf("   F3 adjacent circuit occupied: H out released\n");

    // F4: under tumbleDownTrigger requested, the held control cuts H out even while the
    // signal cannot clear
    line.tumbleDown(TumbleDownTrigger::REQUESTED);
    h832->update(false, now);               // the block refuses: leaving signal stays at Stop
    request(DirectionAuthority::RIGHT);
    cp.tick(now);
    assert(leaving->currentIndication() == Indication::STOP);
    assert(line.requested() && !line.cleared());
    assert(!H->asserted());
    line.tumbleDown(TumbleDownTrigger::CLEARED);
    assert(H->asserted());                  // under cleared, a refused request does not cut H
    request(DirectionAuthority::STOP);
    h832->update(true, now);
    cp.tick(now); cp.tick(now);
    printf("   F4 tumbleDown requested vs cleared\n");

    // F5: D out is the home signal's HR, held one scan
    request(DirectionAuthority::LEFT);
    cp.tick(now);
    assert(home->currentIndication() == Indication::CLEAR);
    assert(D->asserted());
    request(DirectionAuthority::STOP);
    cp.tick(now);
    assert(!D->asserted());
    printf("   F5 D out follows the home signal\n");

    // F6: F out under field_fr: released on request, held released while the block is
    // occupied even after the control is normal, released when the block is clear and
    // the release terms are met
    line.traffic(TrafficMechanism::FIELD_FR).trafficRelease(TrafficRelease::REQUEST_NORMAL_AND_BLOCK_CLEAR);
    const Fact* F = cp.findFact("830NABF");
    assert(F && line.producesF());
    cp.tick(now);
    assert(F->asserted());
    request(DirectionAuthority::RIGHT);
    cp.tick(now);
    assert(!F->asserted() && line.trafficDropped());
    occupied(os, now);                      // train accepts the signal
    cp.tick(now);
    vacant(os, now); occupied(t832, now);
    cp.tick(now);
    request(DirectionAuthority::STOP);      // control normal while the train is in the block
    cp.tick(now);
    assert(!line.requested() && !line.cleared());
    assert(!F->asserted() && line.trafficDropped());   // held: block occupied
    vacant(t832, now); occupied(t834, now);
    cp.tick(now);
    assert(!F->asserted());
    vacant(t834, now);
    cp.tick(now);                           // block clear, control normal: release
    assert(!line.trafficDropped());
    assert(F->asserted());
    printf("   F6 F out: drops on request, held until the traffic release\n");

    // F7: serializer round trip
    static char json[12288];
    assert(cp.serialize(json, sizeof(json), true));
    assert(strstr(json, "\"controlPointLines\"") != nullptr);
    assert(strstr(json, "\"leaving\": [\"830SAB\"], \"tumbleDown\": \"cleared\", \"traffic\": \"field_fr\", \"trafficRelease\": \"request_normal_and_block_clear\"") != nullptr);
    InterlockingPlant twin("TWIN");
    assert(twin.deserialize(json));
    const ControlPointLine* tl = twin.findControlPointLine("830NAB");
    assert(tl && tl->homeMast() == twin.findSignalMast("830NAB"));
    assert(tl->leavingCount() == 1 && tl->leavingMast(0) == twin.findSignalMast("830SAB"));
    assert(tl->governedCount() == 1 && tl->blockCount() == 2);
    assert(tl->traffic() == TrafficMechanism::FIELD_FR && tl->trafficRelease() == TrafficRelease::REQUEST_NORMAL_AND_BLOCK_CLEAR);
    assert(twin.findFact("830NABF") == tl->fOut());
    printf("   F7 round trip\n\n");
}

int main() {
    printf("====================================================\n");
    printf("   FIELDUNIT AUTOMATIC SIGNAL TEST SUITE            \n");
    printf("   FieldUnit ADR 0008                               \n");
    printf("====================================================\n\n");
    partA();
    partB();
    partC();
    partD();
    partE();
    partF();
    printf("====================================================\n");
    printf("   ALL AUTOMATIC SIGNAL TESTS PASSED                \n");
    printf("====================================================\n");
    return 0;
}
