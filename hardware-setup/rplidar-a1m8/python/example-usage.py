# Libraries
from rplidar import RPLidar

# Create lidar
PORT_NAME = "COM7" # <- Check your port name
lidar = RPLidar(PORT_NAME, timeout=1.5)

# Get information 
info = lidar.get_info()
print(info)

# Check health
health = lidar.get_health()
print(health)

# Get 10 scans 
for i, scan in enumerate(lidar.iter_scans()):
    print('%d: Got %d measurments' % (i, len(scan)))
    if i > 10: break

# Safe exit
lidar.stop()
lidar.stop_motor()
lidar.disconnect()