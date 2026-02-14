# mecanum_drive.py
import time
import serial


class MecanumDrive:
    # Motor mapping:
    #   M1 = FL
    #   M2 = FR
    #   M3 = BL
    #   M4 = BR
    #
    # Supports two modes:
    # 1) Full 4-motor firmware:
    #    - set_motors() sends M1..M4 and Teensy drives all motors.
    # 2) Single-wheel mapping firmware (your current Teensy code):
    #    - Select active motor with: S <1..4>
    #    - Only the active motor actually drives
    #    - Bench tests must send ONLY the active motor command

    def __init__(self, port: str, baud: int = 115200, timeout: float = 0.2, settle_s: float = 4.0):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self.settle_s = settle_s

        self.ser = serial.Serial(self.port, self.baud, timeout=self.timeout)
        time.sleep(self.settle_s)

        # True when using Teensy single-wheel mapping firmware
        self.single_wheel_mapping = False

        # Per-motor inversion
        # +1 = normal, -1 = inverted
        # Based on your backward test: right side inverted
        self.inv = [1, 1, 1, 1]  # FL, FR, BL, BR

    def close(self):
        if self.ser and self.ser.is_open:
            try:
                self.stop()
            except Exception:
                pass
            self.ser.close()

    def _send(self, cmd: str):
        self.ser.write((cmd.strip() + "\n").encode("ascii"))
        time.sleep(0.005)

    # Select which virtual motor controls the single physical wheel
    def select_virtual_motor(self, motor_id: int):
        if motor_id < 1 or motor_id > 4:
            raise ValueError("motor_id must be 1..4")
        self._send(f"S {motor_id}")

    # Apply per-motor inversion
    def _apply_inv(self, fl: int, fr: int, bl: int, br: int):
        fl = int(fl * self.inv[0])
        fr = int(fr * self.inv[1])
        bl = int(bl * self.inv[2])
        br = int(br * self.inv[3])
        return fl, fr, bl, br

    # Send commands to all 4 motors (ONLY for full 4-motor firmware)
    def set_motors(self, fl: int, fr: int, bl: int, br: int):
        fl, fr, bl, br = self._apply_inv(fl, fr, bl, br)

        fl = int(max(-255, min(255, fl)))
        fr = int(max(-255, min(255, fr)))
        bl = int(max(-255, min(255, bl)))
        br = int(max(-255, min(255, br)))

        self._send(f"M1 {fl}")
        self._send(f"M2 {fr}")
        self._send(f"M3 {bl}")
        self._send(f"M4 {br}")

    def stop(self):
        if self.single_wheel_mapping:
            for m in (1, 2, 3, 4):
                self.select_virtual_motor(m)
                self._send(f"M{m} 0")
        else:
            self.set_motors(0, 0, 0, 0)

    # Bench helper for Teensy single-wheel mapping firmware
    def bench_send_active(self, motor_id: int, fl: int, fr: int, bl: int, br: int):
        if motor_id < 1 or motor_id > 4:
            raise ValueError("motor_id must be 1..4")

        fl, fr, bl, br = self._apply_inv(fl, fr, bl, br)
        vals = (fl, fr, bl, br)

        v = int(max(-255, min(255, vals[motor_id - 1])))

        self.select_virtual_motor(motor_id)
        self._send(f"M{motor_id} {v}")

    def bench_stop_active(self, motor_id: int):
        if motor_id < 1 or motor_id > 4:
            raise ValueError("motor_id must be 1..4")
        self.select_virtual_motor(motor_id)
        self._send(f"M{motor_id} 0")

    # Logical mecanum movements (inversion handled automatically)
    def forward(self, speed: int = 180):
        self._move(speed, speed, speed, speed)

    def backward(self, speed: int = 180):
        self._move(-speed, -speed, -speed, -speed)

    def right(self, speed: int = 180):
        self._move(speed, -speed, -speed, speed)

    def left(self, speed: int = 180):
        self._move(-speed, speed, speed, -speed)

    def rotate_cw(self, speed: int = 160):
        self._move(speed, -speed, speed, -speed)

    def rotate_ccw(self, speed: int = 160):
        self._move(-speed, speed, -speed, speed)

    def diag_fwd_right(self, speed: int = 180):
        self._move(speed, 0, 0, speed)

    def diag_fwd_left(self, speed: int = 180):
        self._move(0, speed, speed, 0)

    def diag_back_right(self, speed: int = 180):
        self._move(-speed, 0, 0, -speed)

    def diag_back_left(self, speed: int = 180):
        self._move(0, -speed, -speed, 0)

    def _move(self, fl: int, fr: int, bl: int, br: int):
        if self.single_wheel_mapping:
            raise RuntimeError(
                "single_wheel_mapping=True: use bench_send_active() for testing. "
                "Disable mapping and use full firmware for real driving."
            )
        self.set_motors(fl, fr, bl, br)
