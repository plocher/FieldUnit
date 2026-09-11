#ifndef FIELDUNIT_H
#define FIELDUNIT_H

#include "types.h"
#include "SignalAspectPolicy.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "Crossover.h"
#include "SignalControl.h"
#include "SignalMast.h"
#include "ControlTable.h"
#include "ControlPoint.h"
#include "PlantSerializer.h"
#include "WireCodec.h"
#include "CodeLine.h"
#include "FieldUnitConsole.h"

// Core hardware driver interfaces
#include "drivers/IOBit.h"
#include "drivers/IOBus.h"
#include "drivers/TrackCircuitDriver.h"
#include "drivers/SwitchDriver.h"
#include "drivers/SignalMastDriver.h"
#include "drivers/SemaphoreDriver.h"
#include "drivers/CplMastDriver.h"
#include "drivers/MqttApplianceBus.h"
#include "drivers/DriverPolicy.h"

// Specific physical IOBus implementations (CmriIOBus, I2CexpanderIOBus, etc.)
// are leaf headers included individually by the sketches that choose them.

#endif // FIELDUNIT_H
