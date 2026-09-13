#!/usr/bin/env python
"""
test_ctc_desk.py — Python Automated Test Runner for SPCoast cTc Machine

Connects to MQTT (Mosquitto on localhost:1883) and interacts with the physical
cTc desk across the official ADR 0001 CodeLine topics.

Modes:
  python3 tools/test_ctc_desk.py --walk
      Runs an automated lamp walk across all 7 Control Points, publishing
      indication vectors to exercise every switch, track, and signal LED.

  python3 tools/test_ctc_desk.py --monitor
      Monitors incoming controls from the desk. Prints decoded lever
      positions whenever the operator pushes a CODE button.

  python3 tools/test_ctc_desk.py --loopback
      Acts as an automated Ghost Plant for all 7 Control Points:
      Catches control requests, simulates a 2-second switch machine throw,
      and returns matching indications to the desk.
"""

import sys
import time
import argparse
import paho.mqtt.client as mqtt

LAYOUT = "SPCoast"
BROKER_HOST = "localhost"
BROKER_PORT = 1883

STATIONS = {
    "CP_GilroyCaltrain": {
        "normal": "1NWK, (1RWK), 3NWK, (3RWK), (1T1K), (EA1K), (TK1K), (TK2K), (TK3K)",
        "reverse": "(1NWK), 1RWK, (3NWK), 3RWK, 1T1K, EA1K, TK1K, TK2K, TK3K",
    },
    "CP_GilroyInterchange": {
        "normal": "1NWK, (1RWK), 3NWK, (3RWK), (1T1K), (3T1K), (TK1K), (EA1K), (TLK), (TRK)",
        "reverse": "(1NWK), 1RWK, (3NWK), 3RWK, 1T1K, 3T1K, TK1K, EA1K, TLK, TRK",
    },
    "CP_Luchessa": {
        "normal": "1NWK, (1RWK), 3NWK, (3RWK), 5NWK, (5RWK), (1T1K), (3T1K), (2SGK), (2NGK), (2TEK)",
        "reverse": "(1NWK), 1RWK, (3NWK), 3RWK, (5NWK), 5RWK, 1T1K, 3T1K, (2SGK), 2NGK, (2TEK)",
    },
    "CP_Christopher": {
        "normal": "1NWK, (1RWK), 3NWK, (3RWK), 5NWK, (5RWK), (1T1K), (1WAK), (2WAK), (3T1K), (3BT1K), (5T1K), (1EAK), (2EAK), (2SGK), (2NGK), (2TEK)",
        "reverse": "(1NWK), 1RWK, (3NWK), 3RWK, (5NWK), 5RWK, 1T1K, 1WAK, 2WAK, 3T1K, 3BT1K, 5T1K, 1EAK, 2EAK, (2SGK), 2NGK, (2TEK)",
    },
    "CP_Corporal": {
        "normal": "1NWK, (1RWK), 3NWK, (3RWK), (1EAK), (1T1K), (3T1K), (SDTK), (TLK), (TRK), (2SGK), (2NGK), (2TEK)",
        "reverse": "(1NWK), 1RWK, (3NWK), 3RWK, 1EAK, 1T1K, 3T1K, SDTK, TLK, TRK, 2SGK, (2NGK), (2TEK)",
    },
    "CP_Sargent": {
        "normal": "1NWK, (1RWK), (1T1K), (HBDK)",
        "reverse": "(1NWK), 1RWK, 1T1K, HBDK",
    },
    "CP_Watsonville": {
        "normal": "1NWK, (1RWK), (ALTK), (EATK), (SATK), (2SGK), (2NGK), (2TEK)",
        "reverse": "(1NWK), 1RWK, ALTK, EATK, SATK, (2SGK), 2NGK, (2TEK)",
    }
}

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"Connected to Mosquitto MQTT broker at {BROKER_HOST}:{BROKER_PORT}")
        # Subscribe to all SPCoast desk topics
        client.subscribe(f"ctc/{LAYOUT}/codeline/+/controls")
        client.subscribe(f"ctc/{LAYOUT}/codeline/desk/#")
        client.subscribe(f"ctc/{LAYOUT}/telemetry")
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode('utf-8', errors='ignore')
    timestamp = time.strftime("%H:%M:%S")

    # Desk telemetry / status
    if "telemetry" in topic or "info" in topic:
        print(f"[{timestamp}] [DESK TELEMETRY] {topic}: {payload}")
        return

    # Inbound controls from desk
    if "controls" in topic:
        parts = topic.split('/')
        station = parts[3] if len(parts) >= 4 else "Unknown"
        print(f"\n[{timestamp}] ===============================================")
        print(f"[{timestamp}] [CODE RECEIVED] Station: {station}")
        print(f"[{timestamp}] Raw Tokens: {payload}")

        # If running in loopback mode, simulate plant and respond with indications
        if userdata.get("loopback", False):
            simulate_plant_response(client, station, payload)

def simulate_plant_response(client, station, control_payload):
    print(f"  -> Simulating 2.0s switch motor transit at {station}...")
    time.sleep(2.0)

    # Convert control tokens to corresponding indications:
    # 1NWS -> 1NWK, (1RWS) -> (1RWK), 2SGS -> 2SGK, etc.
    ind_tokens = []
    tokens = [t.strip() for t in control_payload.split(',') if t.strip()]

    for t in tokens:
        negated = t.startswith('(') and t.endswith(')')
        clean = t.strip('()')
        if clean.endswith('S'):
            base = clean[:-1] # strip S
            ind_token = base + 'K'
            if negated:
                ind_tokens.append(f"({ind_token})")
            else:
                ind_tokens.append(ind_token)

    # Add mock track circuits (vacant)
    if station in STATIONS:
        # Use declared station track template if available
        resp_payload = ', '.join(ind_tokens)
    else:
        resp_payload = ', '.join(ind_tokens)

    ind_topic = f"ctc/{LAYOUT}/codeline/{station}/indications"
    print(f"  -> Publishing verified indications to {ind_topic}:")
    print(f"     {resp_payload}")
    client.publish(ind_topic, resp_payload, qos=1, retain=True)

def run_lamp_walk(client):
    print("--- Starting Automated Lamp Walk Across All 7 Stations ---")
    for cycle in range(2):
        state_name = "NORMAL + ALL CLEAR" if cycle == 0 else "REVERSE + OCCUPIED"
        key = "normal" if cycle == 0 else "reverse"
        print(f"\n=== CYCLE {cycle + 1}: Setting all stations to {state_name} ===")

        for station, payloads in STATIONS.items():
            topic = f"ctc/{LAYOUT}/codeline/{station}/indications"
            payload = payloads[key]
            print(f"  [{station}] -> {payload}")
            client.publish(topic, payload, qos=1, retain=True)
            time.sleep(0.8) # 800ms stagger between stations

    print("\n--- Lamp Walk Complete ---")

def main():
    parser = argparse.ArgumentParser(description="SPCoast cTc Desk Automated Test Runner")
    parser.add_argument("--host", default=BROKER_HOST, help="MQTT broker host")
    parser.add_argument("--port", type=int, default=BROKER_PORT, help="MQTT broker port")
    parser.add_argument("--walk", action="store_true", help="Run automated lamp walk across all 7 stations")
    parser.add_argument("--monitor", action="store_true", help="Monitor incoming controls from physical desk")
    parser.add_argument("--loopback", action="store_true", help="Run closed-loop ghost plant simulator")

    args = parser.parse_args()

    # Default to monitor if no mode specified
    if not (args.walk or args.monitor or args.loopback):
        args.monitor = True

    userdata = {"loopback": args.loopback}
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="spcoast-test-runner", userdata=userdata)
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(args.host, args.port, 60)
    except Exception as e:
        print(f"Error connecting to broker at {args.host}:{args.port} - {e}")
        print("Make sure Mosquitto is running: 'mosquitto -v' or 'brew services start mosquitto'")
        sys.exit(1)

    client.loop_start()

    if args.walk:
        time.sleep(1.0)
        run_lamp_walk(client)

    if args.monitor or args.loopback:
        mode_str = "Ghost Plant Loopback" if args.loopback else "Live Control Monitor"
        print(f"\n[{mode_str} Active] Press levers and push CODE buttons on the desk.")
        print("Press Ctrl+C to stop.\n")
        try:
            while True:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\nStopping...")

    client.loop_stop()
    client.disconnect()

if __name__ == "__main__":
    main()
