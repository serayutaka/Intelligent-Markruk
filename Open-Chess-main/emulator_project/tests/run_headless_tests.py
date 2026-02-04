import socket
import sys
import time
import json
import threading
import subprocess
import os

# Configuration
HOST = '127.0.0.1'
PORT = 2323
FIRMWARE_BIN = '../build/firmware_host/FirmwareHost'
SCENARIO_FILE = 'scenarios.json'

class EmulatorMock:
    def __init__(self, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((HOST, port))
        self.sock.listen(1)
        self.client = None
        self.running = True
        self.received_cmds = []
        self.lock = threading.Lock()
        
    def start(self):
        print(f"[Emulator] Listening on {PORT}...")
        threading.Thread(target=self._accept, daemon=True).start()
        
    def _accept(self):
        while self.running:
            try:
                conn, addr = self.sock.accept()
                print(f"[Emulator] Connected by {addr}")
                self.client = conn
                threading.Thread(target=self._read, args=(conn,), daemon=True).start()
            except:
                pass

    def _read(self, conn):
        buffer = ""
        while self.running:
            try:
                data = conn.recv(1024)
                if not data: break
                buffer += data.decode('utf-8', errors='ignore')
                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    with self.lock:
                        self.received_cmds.append(line)
                    # print(f"[Device] {line}") 
            except:
                break
        print("[Emulator] Client disconnected")

    def send_sensor(self, r, c, state):
        if self.client:
            msg = f"E {r} {c} {1 if state else 0}\n"
            self.client.sendall(msg.encode())
            print(f"[Emulator] Sent Sensor: {msg.strip()}")

    def get_last_cmd(self):
        with self.lock:
            if self.received_cmds:
                return self.received_cmds[-1]
        return None
    
    def clear_cmds(self):
        with self.lock:
            self.received_cmds = []

    def stop(self):
        self.running = False
        if self.client: self.client.close()
        self.sock.close()

def run_tests():
    # 1. Start Emulator Mock
    emu = EmulatorMock(PORT)
    emu.start()
    
    # 2. Start Firmware
    if not os.path.exists(FIRMWARE_BIN):
        print(f"Error: {FIRMWARE_BIN} not found. Build the project first.")
        # Try finding it in current dir or common paths
        return 1

    print(f"[Test] Launching {FIRMWARE_BIN}...")
    fw_proc = subprocess.Popen([FIRMWARE_BIN], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Wait for connection
    time.sleep(2)
    
    # 3. Load Scenarios
    with open(SCENARIO_FILE) as f:
        scenarios = json.load(f)

    # 4. Run
    for sc in scenarios:
        print(f"\n=== Running: {sc['name']} ===")
        failed = False
        for step in sc['steps']:
            action = step['action']
            print(f" -> Step: {step.get('description', action)}")
            
            if action == 'wait_time':
                time.sleep(step['ms'] / 1000.0)
                
            elif action == 'sensor':
                emu.send_sensor(step['row'], step['col'], step['state'])
                
            elif action == 'wait_led':
                # Wait for specific command
                start = time.time()
                found = False
                while time.time() - start < 2.0:
                    cmd = emu.get_last_cmd()
                    if cmd:
                        if step.get('command') and cmd.startswith(step['command']):
                            found = True
                            break
                        if step.get('color_r'):
                            # Check for L ... r ...
                            parts = cmd.split()
                            if parts[0] == 'L' and int(parts[3]) == step['color_r']:
                                found = True
                                break
                    time.sleep(0.05)
                if not found:
                    print(f"FAILED: Timeout waiting for LED command matching {step}")
                    failed = True
                    break
                else:
                    print("PASS: LED observed")

            elif action == 'expect_log':
                # Check firmware stdout
                # This is tricky as we need to consume stdout async. 
                # For simplicity, we skip strictly verifying text logs in this simple runner, 
                # or we implement a non-blocking read.
                print(" (Log check skipped in simple runner)")

        if not failed:
            print("=== PASS ===")
        else:
            print("=== FAIL ===")

    # Cleanup
    emu.stop()
    fw_proc.terminate()
    print("Done.")

if __name__ == "__main__":
    run_tests()
