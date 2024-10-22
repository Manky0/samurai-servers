all: build_sta build_orquestrator

build_sta:
	g++ sta.cpp ./actions/getPerBeamRSS.cpp ./actions/serverConnect.cpp -o ./outputs/sta.exe

build_orquestrator:
	g++ orquestrator.cpp ./actions/saveToCsv.cpp -o ./outputs/orquestrador.exe

#### CLEAN FILES
clean:
	rm -f outputs/*