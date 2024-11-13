all: build_sta build_orquestrator

build_sta:
	g++ sta.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnect.cpp ./actions/getRealsense.cpp -o ./outputs/sta.exe `pkg-config --cflags --libs opencv4` -lrealsense2

build_orquestrator:
	g++ orquestrator.cpp ./actions/saveToCsv.cpp ./actions/saveToJpeg.cpp -o ./outputs/orquestrador.exe `pkg-config --cflags --libs opencv4`

#### CLEAN FILES
clean:
	rm -f outputs/*