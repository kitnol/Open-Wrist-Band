import serial
import time

NUM_PACKETS = 1
PACKET_SIZE = 1280
TOTAL_BYTES = NUM_PACKETS * PACKET_SIZE
OUTPUT_FILE = "data.bin"

with serial.Serial("COM8", 115200, timeout=60) as s:
    print(f"Waiting for {TOTAL_BYTES} bytes ({NUM_PACKETS} packets)...")
    
    start_time = time.time()
    data = bytearray()
    while len(data) < TOTAL_BYTES:
        chunk = s.read(TOTAL_BYTES - len(data))
        if not chunk:
            print(f"Timeout! Only received {len(data)} / {TOTAL_BYTES} bytes")
            break
        data.extend(chunk)
        print(f"Progress: {len(data)} / {TOTAL_BYTES} bytes")

    elapsed_time = time.time() - start_time
    transfer_speed = len(data) / elapsed_time if elapsed_time > 0 else 0
    
    print(f"Received {len(data)} bytes total")
    print(f"Transfer time: {elapsed_time:.2f} seconds")
    print(f"Transfer speed: {transfer_speed:.2f} bytes/sec ({transfer_speed / 1024:.2f} KB/sec)")
    print(f"First 16 bytes: {data[:16].hex()}")

    with open(OUTPUT_FILE, "wb") as f:
        f.write(data)
    print(f"Saved to {OUTPUT_FILE}")