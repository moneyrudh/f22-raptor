// security.h
#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>
#include <time.h>

// Basic structure to hold score validation data
typedef struct {
    int score;
    uint64_t timestamp;
    char signature[65];  // 32 bytes hex encoded + null terminator
} ScoreValidation;

// Initialize security module
void security_init(void);

// Generate score validation data
ScoreValidation generate_score_validation(int score);

#endif