# Libraries
from rplidar import RPLidar
import pygame
import math 

# Constants
PORT_NAME = "COM7"
LIDAR_TEXTURE_PATH = "./assets/lidar-texture.png"
ICON_PATH = "./assets/sep-icon.png"

FPS = 60
TITLE = "Measurements visualization"
BG_COLOR = (255, 255, 255)
CIRCLE_COLOR = (128, 128, 128)
WINDOW_SIZE = (700, 700)
MAX_DISTANCE = 1000 # 1 meter

# Draw textures
def draw_textures(screen, measurements):
    # Load lidar texture
    lidar_img = pygame.image.load(LIDAR_TEXTURE_PATH)
    lidar_img = pygame.transform.scale(lidar_img, (150, 187))
    
    # Update background color (white)
    screen.fill(BG_COLOR)

    # Draw lidar
    x = (WINDOW_SIZE[0] - lidar_img.get_width()) // 2
    y = (WINDOW_SIZE[1] - lidar_img.get_height()) // 2
    screen.blit(lidar_img, (x, y))

    # Determine imporant variables
    scale = (WINDOW_SIZE[0] / 2) / MAX_DISTANCE
    small_font = pygame.font.SysFont(None, 18)

    # Draw circles
    for i in range(1, 7):
        pygame.draw.circle(screen, CIRCLE_COLOR, (WINDOW_SIZE[0]//2, WINDOW_SIZE[1]//2), 75 + i*100*scale, 1)
        subtitle = small_font.render(f"{i*10}cm", False, CIRCLE_COLOR)
        screen.blit(subtitle, ((WINDOW_SIZE[0] - subtitle.get_width())//2, WINDOW_SIZE[1]//2 - (90+i*100*scale)))

    for _, angle, distance in measurements:
        # Compute position
        print(angle, distance)

        # Draw point
        if distance > 0 and distance <= 600: # Ignore obstacles over 60cm
            # Convert to Radians
            radians = math.radians(angle)
            
            # Scale distance to fit screen
            scale = (WINDOW_SIZE[0] / 2) / MAX_DISTANCE
            
            # Polar to Cartesian conversion
            # Note: Pygame's Y axis is inverted, so we subtract
            x = WINDOW_SIZE[0]//2 + (75 + distance * scale) * math.sin(radians)
            y = WINDOW_SIZE[1]//2 - (75 + distance * scale) * math.cos(radians)

            # Draw a small red point for each measurement
            pygame.draw.circle(screen, (255, 0, 0), (int(x), int(y)), 2)

# Start pygame
pygame.init()
screen = pygame.display.set_mode(WINDOW_SIZE)

pygame.display.set_icon(pygame.image.load(ICON_PATH))
pygame.display.set_caption(TITLE)
clock = pygame.time.Clock()

# Start lidar
lidar = RPLidar(PORT_NAME, timeout=5)

# Get information 
info = lidar.get_info()
print(info)

# Check health
health = lidar.get_health()
print(health)

# Start motor and clear buffer
lidar.clear_input()
lidar.start_motor()
scan_gen = lidar.iter_scans()

# Run application loop
is_running = True
while is_running:
    # Get measurements
    measurements = next(scan_gen)

    # Update obstacle position on the screen
    draw_textures(screen, measurements)

    # Handle events
    for event in pygame.event.get():
        # User clicks X
        if event.type == pygame.QUIT:
            is_running = False
            break
    
    # Complete rendering
    pygame.display.flip()
    clock.tick(FPS)

# Stop lidar
lidar.stop()
lidar.stop_motor()
lidar.disconnect()