#include <tokenBucket.h>

void TokenBucket::setRate(int refillRate)
{
  this->maxTokens = refillRate;
  this->tokens = refillRate;
  this->refillRate = refillRate;
}

bool TokenBucket::consume(int amount)
{
  using namespace std::chrono;

  if (amount > this->tokens)
    return false;

  consumeTime = steady_clock::now();

  this->tokens -= amount;
  return true;
}

int TokenBucket::replenish()
{
  using namespace std::chrono;

  replenishTime = steady_clock::now();

  ellapsedTime = duration_cast<duration<double>>(replenishTime - consumeTime);

  if (ellapsedTime.count() > 1.0)
    refillRate < (maxTokens - tokens) ? tokens += refillRate :
                                        tokens += maxTokens - tokens;

  return tokens;
}
