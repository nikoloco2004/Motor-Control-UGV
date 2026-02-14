from mecanum_drive import MecanumDrive
import time

drive = MecanumDrive(port="COM13", baud=115200)  # change COM port
drive.single_wheel_mapping = False

drive.forward(160)
time.sleep(10)

drive.stop()
drive.close()
