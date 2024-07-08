// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>


#define MAX_SUPPORTED_ALT_FUN_VIPER 14
#define MAX_SUPPORTED_ALT_FUN_INDY  16

typedef struct {
    uint8_t      gpio_number;
} gpio_configuration;


typedef struct {
    uint8_t      alt_mode;
    uint8_t      gpio_no;
    const char   *desc;
} gpio_table_t;

gpio_table_t viper[MAX_SUPPORTED_ALT_FUN_VIPER] = {//Alt mode    // gpio_no    // Alt Funct
                                                       {0,               0,        "Signal Detect 0"},
                                                       {1,               1,        "Signal Detect 1"},
                                                       {2,               2,        "Signal Detect 2"},
                                                       {3,               3,        "Signal Detect 3"},
                                                       {4,               4,        "I2C Clk 0"},
                                                       {5,               5,        "I2C Clk 1"},
                                                       {6,               6,        "I2C Clk 2"},
                                                       {7,               7,        "I2C Clk 3"},
                                                       {8,               8,        "I2C Sda"},
                                                       {9,               9,        "Fast Link Failure"},
                                                       {10,              10,       "1588 load Save"},
                                                       {11,              11,       "1588 PPS"},
                                                       {12,              12,       "1588 SPI CS"},
                                                       {13,              13,       "1588 SPI DO"},
                                                     };
													 
gpio_table_t indy[MAX_SUPPORTED_ALT_FUN_INDY] = {//Alt mode    // gpio_no    // Alt Funct
                                                     {0,               0,        "1588 Event A"},
                                                     {1,               1,        "1588 Event B"},
                                                     {2,               2,        "1588 Ref Clk"},
                                                     {3,               3,        "1588 LD ADJ"},
                                                     {4,               4,        "1588 STI CS N"},
                                                     {5,               5,        "1588 STI CLK"},
                                                     {6,               6,        "1588 STI DO"},
                                                     {7,               7,        "Rcvrd Clk In 1"},
                                                     {8,               8,        "Rcvrd Clk In 2"},
                                                     {9,               9,        "Rcvrd Clk Out 1"},
                                                     {10,              10,       "Rcvrd Clk Out 2"},
                                                     {11,              15,       "SOF 0"},
                                                     {12,              21,       "SOF 1"},
                                                     {13,              16,       "SOF 2"},
                                                     {14,              23,       "SOF 3"},
                                                     {15,              11,       "Port Led"},
                                                    };
