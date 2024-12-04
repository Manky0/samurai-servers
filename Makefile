all: sta ceil ap orq

sta:
	g++-11 sta.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnection.cpp ./actions/getRealsense.cpp -o ./builds/sta.exe `pkg-config --cflags --libs opencv4` -lrealsense2

ceil:
	g++-11 ceil_cam.cpp ./actions/serverConnection.cpp ./actions/getCamFrame.cpp -o ./builds/ceil_cam.exe `pkg-config --cflags --libs opencv4`

ap:
	g++-11 ap.cpp ./actions/serverConnection.cpp ./actions/getCamFrame.cpp -o ./builds/ap.exe `pkg-config --cflags --libs opencv4`

orq:
	g++-11 orquestrator.cpp ./actions/saveToCsv.cpp ./actions/saveToJpeg.cpp -o ./builds/orquestrador.exe `pkg-config --cflags --libs opencv4` -pthread

#### CLEAN FILES
clean:
	rm -f builds/*