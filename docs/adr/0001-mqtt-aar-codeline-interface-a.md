# 0001. MQTT Broker and AAR Tokens for Interface "A"

- Status: accepted
- Date: 2026-09-12

## Context

Centralized Traffic Control (cTc) office machines communicate with trackside control points across Interface "A".
A custom socket or point-to-point protocol requires unique scaffolding for each client and device.
That tight coupling makes testing difficult and prevents third-party integrations.

The architecture requires an open message bus.
The transport must support three operating environments:
1. Physical layouts with microcontrollers and a network broker.
2. Offline development workstations without an external network.
3. Automated test runners in continuous integration pipelines.

FieldUnit Studio also needs a reliable channel to publish control point definitions to field devices.

## Decision

We select an MQTT message broker and symbolic Association of American Railroads (AAR) text tokens for Interface "A".

### Topic Structure

The system uses this topic hierarchy:
`/layout/<layoutname>/codeline/<controlpointname>/<channel>`

The hierarchy defines five channels:
1. `controls`: Transmits dispatcher requests. Messages are not retained.
2. `indications`: Transmits field plant states. Messages are retained (QoS 1).
3. `json`: Transmits the plant specification JSON. Messages are retained (QoS 1).
4. `telemetry`: Transmits heartbeat and operational health. The Last Will and Testament marks offline state.
5. `info`: Transmits hardware mappings, firmware versions, and diagnostic logs.

### Payload Encoding

The system encodes plant controls and indications as comma-separated AAR text tokens:
- Control tokens end with `S` (such as `1NWS`, `(1RWS)`, `2NGS`, `2HS`).
- Indication tokens end with `K` (such as `1NWK`, `(1RWK)`, `1T1K`, `2NGK`, `2TEK`).
- An unparenthesized token means asserted or true.
- A parenthesized token means unasserted or false.

### Boundary of Responsibilities

FieldUnit Studio acts as an operator console and design tool.
Studio publishes authoritative plant specifications to the `json` topic.
Studio does not simulate vital interlocking rules.
FieldUnit executes all vital interlocking logic, route locking, and detector rules.

## Consequences

- FieldUnit Studio and field devices require an MQTT client library.
- Offline workstations must run a local broker or an embedded broker inside Studio.
- New consoles immediately receive plant truth by reading retained indication topics.
- External software can monitor or control the layout through standard MQTT topics.
