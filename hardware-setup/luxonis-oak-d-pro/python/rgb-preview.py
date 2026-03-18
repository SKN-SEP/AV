import cv2
import depthai as dai

# Create pipeline and camera
pipeline = dai.Pipeline()
cam = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_A)
outputQueue = cam.requestOutput(size=(1920, 1080), fps=60).createOutputQueue()

# Connect to device and start pipeline
pipeline.start()
while pipeline.isRunning():
    frame = outputQueue.get()
    cv2.imshow("rgb", frame.getCvFrame())

    if cv2.waitKey(1) == ord('q'):
        break
cv2.destroyAllWindows()