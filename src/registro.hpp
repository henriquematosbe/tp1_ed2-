//Registros e Metricas
#ifndef REGISTRO_H
#define REGISTRO_H


  // registro do arquivo binario
  struct Registro{
    int chave;
    long dado1;
    char dado2[5000];


  };
  //metricas de cada etapa
  struct Metricas{
    long transferencia = 0;// n de transferencias
    long comparacoes = 0; // n de comparacoes
    double tempo = 0.0; //seg

  };




#endif // REGISTRO_H