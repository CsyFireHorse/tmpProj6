// TokenBucket.h
#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <chrono>
#include <cstdint>

class TokenBucket {
public:
    // Constructor with fraction (numerator/denominator)
    TokenBucket(unsigned int token_rate_num = 10, unsigned int token_rate_den = 1) 
        : token_rate_num_(token_rate_num )
        , token_rate_den_(token_rate_den)
    {}
    
    int getCycles() {
        auto current_time = std::chrono::steady_clock::now();
        
        if (!initialized_) {
            last_update_time_ = current_time;
            initialized_ = true;
            return 0;
        }
        
        // Calculate elapsed time in microseconds (integer)
        auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - last_update_time_
        ).count();
        
        last_update_time_ = current_time;
        
        if (elapsed_us <= 0) {
            return 0;
        }
        
        // Accumulate: elapsed_us * token_rate_num_
        // No division here, just accumulation
        accumulated_ += static_cast<int64_t>(elapsed_us) * token_rate_num_;
        
        // Calculate cycles and residue using integer division and modulo
        int cycles = static_cast<int>(accumulated_ / token_rate_den_);
        accumulated_ %= token_rate_den_;  // Keep residue (remainder)
        
        return cycles;
    }
    
    void setTokenRate(unsigned int num, unsigned int den) {
        token_rate_num_ = num;
        token_rate_den_ = den;
    }
    
    void reset() {
        accumulated_ = 0;
        initialized_ = false;
    }
    
    // Get residue (remainder) as integer
    int64_t getResidue() const {
        return accumulated_;
    }

private:
    unsigned int token_rate_num_;     // Token rate numerator
    unsigned int token_rate_den_;     // Token rate denominator
    int64_t accumulated_ = 0;    // Accumulated value: elapsed_time * token_rate_num (residue after cycles extraction)
    std::chrono::steady_clock::time_point last_update_time_;
    bool initialized_ = false;
};

#endif // TOKEN_BUCKET_H