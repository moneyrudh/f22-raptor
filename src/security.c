// security.c
#include "security.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Get key from environment during build
static const unsigned char* get_secret_key(void) {
    printf("Debug: Checking for secret key...\n");
    #ifdef SCORE_SECRET_KEY
        printf("Debug: SCORE_SECRET_KEY is defined!\n");
        #define STRINGIZE(x) #x
        #define STRINGIZE_VALUE_OF(x) STRINGIZE(x)
        printf("Debug: Key value is: %s\n", STRINGIZE_VALUE_OF(SCORE_SECRET_KEY));
        return (unsigned char*)STRINGIZE_VALUE_OF(SCORE_SECRET_KEY);
    #endif
    printf("Debug: NO SECRET KEY DEFINED :(\n");
    
    return NULL;
}


// Simple XOR-based HMAC for demo (replace with real crypto in prod)
static void generate_signature(int score, uint64_t timestamp, char* output) {
    char data[256];
    snprintf(data, sizeof(data), "%d:%llu", score, timestamp);
    
    // Store the key in a local array to ensure it stays valid
    const unsigned char secret_key[] = SCORE_SECRET_KEY;
    
    unsigned char result[32] = {0};
    size_t data_len = strlen(data);
    size_t key_len = strlen((char*)secret_key);
    
    for(size_t i = 0; i < 32; i++) {
        result[i] = data[i % data_len] ^ secret_key[i % key_len];
    }
    
    for(size_t i = 0; i < 32; i++) {
        sprintf(output + (i * 2), "%02x", result[i]);
    }
}

#ifdef __EMSCRIPTEN__
// Add this helper function to get Unix timestamp in JS
EM_JS(double, get_unix_timestamp, (), {
    return Math.floor(Date.now() / 1000);
});
#endif

ScoreValidation generate_score_validation(int score) {
    ScoreValidation validation;
    validation.score = score;
    
    #ifdef __EMSCRIPTEN__
    validation.timestamp = (int32_t)get_unix_timestamp();
    #else
    validation.timestamp = (int32_t)time(NULL);
    #endif
    
    generate_signature(score, validation.timestamp, validation.signature);
    return validation;
}