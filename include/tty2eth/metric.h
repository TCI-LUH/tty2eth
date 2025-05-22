#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include "lwip.h"


typedef struct {
    float voltage;
    float shuntVoltage;
    float current;
    float power;
} PowerMetric;

typedef struct {
    ip_addr_t ip;
    uint32_t netDuplexMode;
    int netSpeed;
    float temp;
    PowerMetric power3v3;
    PowerMetric power5v;
    PowerMetric power1v2;
    time_t uptimeSince;
    bool hasPhytecPower;
} Metric;


void initMetric();
Metric* getCurrentMetric();