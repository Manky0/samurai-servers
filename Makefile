all: build_sta build_orquestrator

build_sta:
	g++ sta.cpp getPerBeamRSS.cpp -o ./outputs/sta.exe

build_orquestrator:
	g++ orquestrator.cpp -o ./outputs/orquestrador.exe

#### CLEAN FILES
clean:
	rm -f outputs/*