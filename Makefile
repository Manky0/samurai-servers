all: build_sta build_orquestrator

build_sta:
	g++ sta.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnect.cpp ./actions/getCamFrame.cpp -o ./outputs/sta.exe `pkg-config --cflags --libs opencv4`

build_orquestrator:
	g++ orquestrator.cpp ./actions/saveToCsv.cpp ./actions/saveToJpeg.cpp -o ./outputs/orquestrador.exe `pkg-config --cflags --libs opencv4`

#### CLEAN FILES
clean:
	rm -f outputs/*