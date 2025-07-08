all: sta ceil ap orch

sta:
	g++-11 sta.cpp ./actions/controlRobot.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnection.cpp ./actions/getRealsense.cpp -o ./builds/sta.exe `pkg-config --cflags --libs opencv4` -lrealsense2

ceil:
	g++-11 ceil_cam.cpp ./actions/serverConnection.cpp ./actions/getCamFrame.cpp -o ./builds/ceil_cam.exe `pkg-config --cflags --libs opencv4`

ap:
	g++-11 ap.cpp ./actions/serverConnection.cpp ./actions/getCamFrame.cpp -o ./builds/ap.exe `pkg-config --cflags --libs opencv4`

orch:
	g++-11 -std=c++17 orchestrator.cpp ./actions/saveToCsv.cpp ./actions/saveToJpeg.cpp -o ./builds/orchestrator.exe `pkg-config --cflags --libs opencv4` -pthread

#### CLEAN FILES
clean:
	rm -f builds/*