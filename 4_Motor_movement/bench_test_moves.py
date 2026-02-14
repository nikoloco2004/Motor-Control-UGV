# bench_test_moves.py
import time
from mecanum_drive import MecanumDrive

PORT = "COM13"

SPEED = 180
TURN_SPEED = 160
MOVE_TIME = 1.0
TURN_90_TIME = 0.55
TURN_180_TIME = 1.10


def expected_text(fl: int, fr: int, bl: int, br: int) -> str:
    def one(name: str, v: int) -> str:
        if v > 0:
            return f"{name} forward ({v})"
        if v < 0:
            return f"{name} backward ({abs(v)})"
        return f"{name} stop (0)"

    return "Expecting: " + ", ".join([
        one("FR", fr),
        one("FL", fl),
        one("BL", bl),
        one("BR", br),
    ])


def bench_test_with_print(drv: MecanumDrive, name: str, fl: int, fr: int, bl: int, br: int, duration: float):
    print(f"\n=== Testing {name} ===")
    print(expected_text(fl, fr, bl, br))

    afl, afr, abl, abr = drv._apply_inv(fl, fr, bl, br)
    print(f"After inversion: FL={afl}, FR={afr}, BL={abl}, BR={abr}")

    for motor_id, label in [
        (1, "FL (M1)"),
        (2, "FR (M2)"),
        (3, "BL (M3)"),
        (4, "BR (M4)"),
    ]:
        print(f"  -> Bench step: selecting {label}")
        drv.bench_send_active(motor_id, fl, fr, bl, br)
        time.sleep(duration)
        drv.bench_stop_active(motor_id)
        time.sleep(0.4)


drv = MecanumDrive(port=PORT, baud=115200)
drv.single_wheel_mapping = True

try:
    # FORWARD
    # bench_test_with_print(drv, "FORWARD", SPEED, SPEED, SPEED, SPEED, MOVE_TIME)

    # BACKWARD
    # bench_test_with_print(drv, "BACKWARD", -SPEED, -SPEED, -SPEED, -SPEED, MOVE_TIME)

    # STRAFE RIGHT
    bench_test_with_print(drv, "STRAFE RIGHT", SPEED, -SPEED, -SPEED, SPEED, MOVE_TIME)

    # STRAFE LEFT
    # bench_test_with_print(drv, "STRAFE LEFT", -SPEED, SPEED, SPEED, -SPEED, MOVE_TIME)

    # ROTATE CW
    # bench_test_with_print(drv, "ROTATE CW", TURN_SPEED, -TURN_SPEED, TURN_SPEED, -TURN_SPEED, MOVE_TIME)

    # ROTATE CCW
    # bench_test_with_print(drv, "ROTATE CCW", -TURN_SPEED, TURN_SPEED, -TURN_SPEED, TURN_SPEED, MOVE_TIME)

    # TURN 90 CW
    # bench_test_with_print(drv, "TURN 90 CW", TURN_SPEED, -TURN_SPEED, TURN_SPEED, -TURN_SPEED, TURN_90_TIME)

    # TURN 180 CW
    # bench_test_with_print(drv, "TURN 180 CW", TURN_SPEED, -TURN_SPEED, TURN_SPEED, -TURN_SPEED, TURN_180_TIME)

    # DIAGONALS
    # bench_test_with_print(drv, "DIAG FWD-RIGHT", SPEED, 0, 0, SPEED, MOVE_TIME)
    # bench_test_with_print(drv, "DIAG FWD-LEFT", 0, SPEED, SPEED, 0, MOVE_TIME)
    # bench_test_with_print(drv, "DIAG BACK-RIGHT", -SPEED, 0, 0, -SPEED, MOVE_TIME)
    # bench_test_with_print(drv, "DIAG BACK-LEFT", 0, -SPEED, -SPEED, 0, MOVE_TIME)

finally:
    drv.stop()
    drv.close()
