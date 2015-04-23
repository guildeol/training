/*!
 * \file tokenBucket.h
 * \brief Cabecalho para classe TokenBucket.
 *
 * \date 23/04/2015
 * \author Guilherme Costa <glhrmcosta91@gmail.com>
 */

#ifndef TOKENBUCKET_H
#define TOKENBUCKET_H

/*!
 * \class TokenBucket
 * \brief Classe para implementacao do algoritmo token bucket de controle de
 *        taxa de transferencia de dados.
 */
class TokenBucket
{

public:

  /*!
   * \brief Construtor da classe.
   * \param[in] maxTokens Quantidade maxima de token que o balde suporta.
   * \param[in] outRate taxa (em bytes/seg) de transferencia desejada.
   * \param[in] refillRate Quantidade de tokens repostos por segundo.
   */
  TokenBucket(int maxTokens, int outRate, int refillRate);

  /*!
   * \brief Tenta consumir uma certa quantidade de tokens.
   * \param[in] amount Quantidade de tokens a ser consumida.
   * \return Retorna true caso tenha consumido os tokens. false caso contrario.
   */
  bool consume(int amount);

  /*!
   * \brief Coloca refillRate tokens no balde.
   * \return Quantidade atual de tokens no balde.
   */
  int replenish();

private:

  int maxTokens = 0;
  int outRate = 0;
  int refillRate = 0;
  int tokens = 0;

};

#endif
