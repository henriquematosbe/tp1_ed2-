CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

FONTES  = $(wildcard src/*.cpp)
OBJETOS = $(FONTES:.cpp=.o)
METODOS = $(filter-out src/main.o, $(OBJETOS))

all: pesquisa experimentos

pesquisa: $(OBJETOS)
	$(CXX) $(CXXFLAGS) -o pesquisa $(OBJETOS)

experimentos: tools/experimentos.cpp $(METODOS)
	$(CXX) $(CXXFLAGS) -o experimentos tools/experimentos.cpp $(METODOS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o pesquisa experimentos
