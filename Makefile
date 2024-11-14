all: build_sta build_ceil build_orquestrator

build_sta:
	g++ sta.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnection.cpp ./actions/getRealsense.cpp -o ./builds/sta.exe `pkg-config --cflags --libs opencv4` -lrealsense2

build_ceil:
	g++ ceil_cam.cpp ./actions/serverConnection.cpp ./actions/getCamFrame.cpp -o ./builds/ceil_cam.exe `pkg-config --cflags --libs opencv4`

build_orquestrator:
	g++ orquestrator.cpp ./actions/saveToCsv.cpp ./actions/saveToJpeg.cpp -o ./builds/orquestrador.exe `pkg-config --cflags --libs opencv4`

#### CLEAN FILES
clean:
	rm -f builds/*