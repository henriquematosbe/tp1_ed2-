#ifndef REGISTRO_HPP
#define REGISTRO_HPP

// Registro armazenado apenas no arquivo binario
struct Registro {
    int chave;
    long dado1;
    char dado2[5000];
};

// Nome curto usado nos metodos 1 e 2
typedef Registro tipoitem;

// Metricas de uma etapa (criacao do indice ou pesquisa)
struct Metricas {
    long transferencias = 0; // leituras da memoria externa
    long comparacoes = 0;    // comparacoes entre chaves
    double tempo = 0.0;      // segundos
};

#endif
