import cv2
import depthai as dai

# Create pipeline and camera
pipeline = dai.Pipeline()
camB = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_B)
bOutputQueue = camB.requestOutput(size=(1280, 720), fps=120).createOutputQueue()

camC = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_C)
cOutputQueue = camC.requestOutput(size=(1280, 720), fps=120).createOutputQueue()

# Connect to device and start pipeline
pipeline.start()
while pipeline.isRunning():
    frameB = bOutputQueue.get()
    frameC = cOutputQueue.get()

    cv2.imshow("Mono camera (B)", frameB.getCvFrame())
    cv2.imshow("Mono camera (C)", frameC.getCvFrame())

    if cv2.waitKey(1) == ord('q'):
        break
cv2.destroyAllWindows()