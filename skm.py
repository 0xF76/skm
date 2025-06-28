import serial
import struct
import threading
import time


# ------------------------
# CRC Computation (CRC-16-CCITT)
# ------------------------
def crc_compute(payload: bytes, poly=0x1021, init_val=0xFFFF) -> int:
    crc = init_val
    for byte in payload:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF  # Ensure 16-bit result
    return crc

# ------------------------
# Build Packet: CMD + LEN + PAYLOAD + CRC
# ------------------------
def build_packet(cmd: int, payload_bytes: bytes) -> tuple[bytes, int]:
    length = len(payload_bytes)
    crc = crc_compute(payload_bytes)
    packet = struct.pack('<BB', cmd, length) + payload_bytes + struct.pack('<H', crc)
    return packet, crc

# ------------------------
# Pretty Print Packet
# ------------------------
def print_packet(prefix: str, cmd: int, length: int, payload: bytes, crc: int, calc_crc: int, hex_view: str):
        print(f"\n--- {prefix} Packet ---")
        print(f"CMD    : 0x{cmd:02X}")
        print(f"LEN    : {length}")
        print(f"PAYLOAD: {bytes(payload)}")
        print(f"CRC    : 0x{crc:04X} (calculated: 0x{calc_crc:04X})")
        print(f"HEX    : {hex_view}")
        print("-----------------------")

# ------------------------
# Send Packet over Serial
# ------------------------
def send_packet(ser: serial.Serial, cmd: int, payload: bytes):
    packet, crc = build_packet(cmd, payload)
    calc_crc = crc_compute(payload)
    print_packet("Sending", cmd, len(payload), payload, crc, calc_crc, packet.hex())
    ser.write(packet)

# ------------------------
# Receive & Parse Packets
# ------------------------
recv_buffer = bytearray()
pong_received = threading.Event()
waiting_for_pong = False

def read_loop(ser: serial.Serial):
    global recv_buffer, pong_received

    while True:
        try:
            data = ser.read(ser.in_waiting or 1)
            if not data:
                time.sleep(0.1)  # Avoid busy waiting
                continue
            
            recv_buffer.extend(data)

            while len(recv_buffer) >= 4:  # Minimum packet size: CMD + LEN + CRC
                cmd = recv_buffer[0]
                length = recv_buffer[1]
                expected_size = 2 + length + 2
                if len(recv_buffer) < expected_size:
                    break # Wait for more bytes
                packet = recv_buffer[:expected_size]
                recv_buffer = recv_buffer[expected_size:]

                payload = packet[2:2+length]
                crc = struct.unpack('<H', packet[-2:])[0]
                calc_crc = crc_compute(payload)

                if cmd == 0x06 and length == 4 and payload == b'PONG':
                    pong_received.set()
                
                print_packet("Received", cmd, length, payload, crc, calc_crc, packet.hex())
        
        except serial.SerialException as e:
            print(f"Serial error: {e}")
            break


# ------------------------
# Menu
# ------------------------
def menu():
    print("\nChoose option:")
    print("[1] PING (0x06)")
    print("[2] LED (0x05)")
    print("[3] Custom Packet")
    print("[q] Quit")



# ------------------------
# Main loop
# ------------------------
def main():
    global waiting_for_pong, pong_received
    port = '/dev/ttyACM0'
    ser = serial.Serial(port, baudrate=115200, timeout=1)

    threading.Thread(target=read_loop, args=(ser,), daemon=True).start()

    while True:
        try:
            if waiting_for_pong:
                break

            menu()
            choice = input("> ").strip()

            if choice.lower() == 'q':
                print("Exiting...")
                break
            elif choice == '1':
                waiting_for_pong = True
                pong_received.clear()
                send_packet(ser, 0x06, b'PING')
                
                pong_received.wait(0.1)
                waiting_for_pong = False
            elif choice == '2':
                index = int(input("Enter LED index (0 - 6): "))
                r = int(input("Enter R value (0-255): "))
                g = int(input("Enter G value (0-255): "))
                b = int(input("Enter B value (0-255): "))
                send_packet(ser, 0x05, struct.pack('<BBBB', index, r, g, b))
            elif choice == '3':
                cmd_hex = input("Enter CMD (hex, e.g., 0x42): ").strip()                
                
                if cmd_hex.startswith('0x'):
                    cmd = int(cmd_hex, 16)
                else:
                    cmd = int(cmd_hex)

                payload_hex = input("Enter PAYLOAD (hex, e.g., 01 02 03): ").strip()
                payload_bytes = bytes.fromhex(payload_hex)

                send_packet(ser, cmd, payload_bytes)
        except (ValueError, KeyboardInterrupt):
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")            
    
    ser.close()
    print("Serial port closed.")


# ------------------------
# Entry point
# ------------------------
if __name__ == "__main__":
    main()

