// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


/* Malibu10G Part Numbers */
#define PHY_TYPE_8256   0x8256
#define PHY_TYPE_8257   0x8257
#define PHY_TYPE_8258   0x8258

/* Viper PHY Part Numbers */
#define PHY_TYPE_8582   8582
#define PHY_TYPE_8584   8584
#define PHY_TYPE_8575   8575
#define PHY_TYPE_8564   8564
#define PHY_TYPE_8562   8562
#define PHY_TYPE_8586   8586

/* Tesla PHY Part Numbers */
#define PHY_TYPE_8574   8574
#define PHY_TYPE_8504   8504
#define PHY_TYPE_8572   8572
#define PHY_TYPE_8552   8552

/* LAN8814 PHY Part Number */
#define PHY_TYPE_8814   8814


typedef enum {
    PHY_FAMILY_MALIBU_10G,
    PHY_FAMILY_VIPER,
    PHY_FAMILY_TESLA,
    PHY_FAMILY_LAN8814,
    PHY_FAMILY_MALIBU_25G,
} phy_family_t;

typedef struct {
    phy_family_t family;
} demo_phy_info_t;



mepa_rc phy_family_detect(meba_inst_t meba_instance, mepa_port_no_t port_no, demo_phy_info_t *phy_info);

mepa_rc mepa_dev_create_check(meba_inst_t meba_instance, mepa_port_no_t port_no);

int atoi_Conversion(const char* strg);
