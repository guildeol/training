#include <tokenBucket.h>

TokenBucket::TokenBucket(int maxTokens, int outRate, int refillRate)
{
  this->maxTokens = maxTokens;
  this->tokens = maxTokens;
  this->outRate = outRate;
  this->refillRate = refillRate;
}

bool TokenBucket::consume(int amount)
{
  if (amount > this->tokens)
    return false;

  this->tokens -= amount;
  return true;
}

int TokenBucket::replenish()
{
  refillRate < (maxTokens - tokens) ? tokens += refillRate :
                                      tokens += maxTokens - tokens;

  return tokens;
}
