import cv2
import depthai as dai
import numpy as np

# Create camera and configure left and right cameras
pipeline = dai.Pipeline()
monoLeft = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_B)
monoRight = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_C)
stereo = pipeline.create(dai.node.StereoDepth)

# Linking
monoLeftOut = monoLeft.requestFullResolutionOutput()
monoRightOut = monoRight.requestFullResolutionOutput()
monoLeftOut.link(stereo.left)
monoRightOut.link(stereo.right)

# Robust settings
stereo.setRectification(True)
stereo.setExtendedDisparity(True)
stereo.setLeftRightCheck(True)

disparityQueue = stereo.disparity.createOutputQueue()

colorMap = cv2.applyColorMap(np.arange(256, dtype=np.uint8), cv2.COLORMAP_JET)
colorMap[0] = [0, 0, 0]  # to make zero-disparity pixels black

# Connect to device and start pipeline
with pipeline:
    pipeline.start()
    maxDisparity = 1
    while pipeline.isRunning():
        disparity = disparityQueue.get()
        assert isinstance(disparity, dai.ImgFrame)
        npDisparity = disparity.getFrame()
        maxDisparity = max(maxDisparity, np.max(npDisparity))
        colorizedDisparity = cv2.applyColorMap(((npDisparity / maxDisparity) * 255).astype(np.uint8), colorMap)
        cv2.imshow("disparity", colorizedDisparity)
        key = cv2.waitKey(1)
        if key == ord('q'):
            pipeline.stop()
            break