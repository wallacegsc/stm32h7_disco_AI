"""

claudeAI

collection.py — MPU6050 data collector over the firmware's serial menu (STM32).

Flow:
    The firmware exposes a serial menu. The `motion_get_batch_csv` command makes
    the sensor emit ONE window of 50 samples, delimited by markers:

        Csv Start <ax,ay,az,gx,gy,gz>
        +0.1575,+0.0042,+1.0361,-0.5488,+2.5610,-0.2744
        ... (50 lines) ...
        Csv End <ax,ay,az,gx,gy,gz>

    This script calls the command N times, once per window, and writes each
    window as ONE JSONL line to the output file (append mode).

Output format (JSONL — one window per line):

    {"label": "Idle", "samples": [[ax,ay,az,gx,gy,gz], ... 50x ...]}
    {"label": "Wave", "samples": [[ax,ay,az,gx,gy,gz], ... 50x ...]}

    - Each line is an independent JSON object -> resilient to Ctrl+C (loses at
      most the in-progress window, never corrupts the whole file).
    - No timestamp: the rate is fixed (50 samples/second), so the sample index
      within the array IS the time (position i = i * 20 ms).
    - Columns: ax, ay, az (accelerometer, g) | gx, gy, gz (gyroscope, deg/s).

Usage:
    python collection.py --port COM10 --label Idle --out data.jsonl --windows 20
"""

import argparse
import json
import time

import serial as ser


# Valid labels for collection. Restricting --label to this set prevents
# recording a dataset with mistyped labels ("idle" vs "Idle").
LABELS = ["Idle", "Circle", "Punch", "Wave"]


class InterfaceMenuSerial:
    """Low-level channel to the firmware's serial menu.

    Single responsibility: send commands and read lines over the serial port.
    It knows nothing about the data format or the notion of a "window" — that
    is MotionSensorCollection's job, which uses this class via composition.
    """

    def __init__(self, port: str, baud: int):
        self.port = port
        self.baud = baud
        # timeout=2: readline() waits at most 2 s and returns whatever it has.
        # Without it, a read would block forever if the firmware went silent.
        self.s = ser.Serial(port=port, baudrate=baud, timeout=2)

    def write_cmd(self, cmd: str):
        """Send a command terminated by '\\r' (the end-of-line the menu expects)."""
        self.s.write((cmd + "\r").encode())
        self.s.flush()  # make sure the command leaves before moving on

    def interrupt_menu(self):
        """Send Ctrl+C (0x03) — the firmware clears its buffer and aborts loops."""
        self.s.write(b"\x03")
        self.s.flush()

    def clear_input(self):
        """Drop bytes already received but unread (echo, prompt, boot, self-test)."""
        self.s.reset_input_buffer()

    def read_line(self) -> bytes:
        """Read one line (up to '\\n' or timeout). Returns b'' on timeout."""
        return self.s.readline()

    def close(self):
        """Release the port. Forgetting this leaves the COM busy for the next run."""
        if self.s.is_open:
            self.s.close()


class MotionSensorCollection:
    """Collects 50-sample windows from the MPU6050 and writes them as JSONL.

    Specialized: it knows the firmware command, the window markers, and the
    output format. It delegates all serial I/O to InterfaceMenuSerial.
    """

    CMD = "motion_get_batch_csv"       # command that emits one CSV window
    START = "Csv Start"                # start marker (in the firmware stream)
    END = "Csv End"                    # end marker
    WINDOW = 50                        # samples per window (fixed)

    def __init__(self, interface: InterfaceMenuSerial, label: str, out_path: str):
        self.io = interface
        self.label = label
        self.out_path = out_path

    def _sync(self):
        """Bring the firmware menu to a clean, known state before collecting."""
        self.io.write_cmd("")     # lone '\r': closes any dangling/partial line
        time.sleep(0.2)           # let the firmware process and return to Waiting
        self.io.clear_input()     # drop the echo/prompt that just arrived

    def _read_window(self) -> list:
        """Request one window from the firmware and return its samples.

        Returns an empty list if the window does not arrive complete
        (timeout, noise, etc.).
        """
        self._sync()
        self.io.write_cmd(self.CMD)

        samples = []
        capturing = False
        while True:
            raw = self.io.read_line()
            if not raw:                       # timeout: firmware silent -> abort
                return []
            line = raw.decode(errors="ignore").strip()

            if self.START in line:            # entered the data block
                capturing = True
                continue
            if self.END in line:              # end of block -> return what we got
                return samples
            if not capturing:                 # echo, prompt, self-test -> ignore
                continue

            # inside the block: expect "float,float,float,float,float,float"
            try:
                vals = [float(x) for x in line.split(",")]
                if len(vals) == 6:
                    samples.append(vals)
            except ValueError:
                pass                          # malformed line -> drop, don't crash

    def collect(self, n_windows: int):
        """Collect n windows, writing each as one JSONL line (append).

        Ctrl+C ends cleanly: whatever was already written stays safe on disk.
        """
        written = 0
        try:
            # "a" = append: merges with previous collections in the same file.
            with open(self.out_path, "a", encoding="utf-8") as f:
                for i in range(n_windows):
                    win = self._read_window()

                    # Only accept a complete window. A bad take (sensor
                    # disconnected, dropped line) is discarded instead of
                    # misaligning the rest of the dataset.
                    if len(win) != self.WINDOW:
                        print(f"[{i + 1}/{n_windows}] dropped ({len(win)} samples)")
                        continue

                    record = {"label": self.label, "samples": win}
                    f.write(json.dumps(record) + "\n")
                    f.flush()  # persist now — Ctrl+C won't lose a finished window
                    written += 1
                    print(f"[{i + 1}/{n_windows}] ok  ({self.label})")

        except KeyboardInterrupt:
            print("\nInterrupted by user.")
        finally:
            self.io.interrupt_menu()  # send 0x03 to stop anything in the firmware
            print(f"Wrote {written} windows to '{self.out_path}'.")


def parse_args():
    p = argparse.ArgumentParser(description="MPU6050 collector (serial menu -> JSONL)")
    p.add_argument("--port", required=True,
                   help="Serial port (e.g. COM10, /dev/ttyACM0)")
    p.add_argument("--label", required=True, choices=LABELS,
                   help=f"Collection label. Options: {', '.join(LABELS)}")
    p.add_argument("--out", required=True,
                   help="Output .jsonl file (append mode)")
    p.add_argument("--baud", type=int, default=460800,
                   help="Baud rate (default: 460800)")
    p.add_argument("--windows", type=int, default=20,
                   help="Number of windows to collect (default: 20)")
    return p.parse_args()


def main():
    args = parse_args()
    print(f"port={args.port} label={args.label} out={args.out} "
          f"baud={args.baud} windows={args.windows}")

    menu = InterfaceMenuSerial(args.port, args.baud)
    try:
        collector = MotionSensorCollection(menu, args.label, args.out)
        collector.collect(args.windows)
    finally:
        # close the port in every case (error, Ctrl+C, normal end)
        menu.close()


if __name__ == "__main__":
    main()