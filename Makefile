CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

FONTES  = $(wildcard src/*.cpp)
OBJETOS = $(FONTES:.cpp=.o)

all: pesquisa gerar

pesquisa: $(OBJETOS)
	$(CXX) $(CXXFLAGS) -o pesquisa $(OBJETOS)

gerar: tools/gerar.cpp src/arquivo.cpp
	$(CXX) $(CXXFLAGS) -o gerar tools/gerar.cpp src/arquivo.cpp

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o pesquisa gerar
