// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <microchip/ethernet/board/api.h>
#include <vtss_phy_api.h>
#include "meba_aux.h"
#include "meba_generic.h"


typedef struct
{
    vtss_gpio_10g_no_t gpio_tx_dis;   /* Tx Disable GPIO number */
    vtss_gpio_10g_no_t gpio_aggr_int; /* Aggregated Interrupt-0 GPIO number */
    vtss_gpio_10g_no_t gpio_i2c_clk;  /* GPIO Pin selection as I2C_CLK for I2C communication with SFP  */
    vtss_gpio_10g_no_t gpio_i2c_dat;  /* GPIO Pin selection as I2C_DATA for I2C communication with SFP */
    vtss_gpio_10g_no_t gpio_virtual;  /* Per port Virtual GPIO number,for internal GPIO usage          */
    vtss_gpio_10g_no_t gpio_sfp_mod_det;  /* GPIO Pin selection as SFP module detect              */
    uint32_t           aggr_intrpt;   /* Channel interrupt bitmask */
}fa_malibu_gpio_port_map_t;

static const fa_malibu_gpio_port_map_t malibu_gpio_map[] = {
    /* P49: CH3 */
    {
        28, 34, 27, 26, 0, 25,
        ((1<<VTSS_10G_GPIO_AGGR_INTRPT_CH3_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_0_INTR3_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_1_INTR3_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_GPIO_INTR_EN)),
    },
    /* P50: CH2 */
    {
        20, 34, 19, 18, 0, 17,
        ((1<<VTSS_10G_GPIO_AGGR_INTRPT_CH2_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_0_INTR2_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_1_INTR2_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_GPIO_INTR_EN)),
    },
    /* P51: CH1 */
    {
        12, 34, 11, 10, 0, 9,
        ((1<<VTSS_10G_GPIO_AGGR_INTRPT_CH1_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_0_INTR1_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_1_INTR1_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_GPIO_INTR_EN)),

    },
    /* P52: CH0 */
    {
        4, 34, 3, 2, 0, 1,
        ((1<<VTSS_10G_GPIO_AGGR_INTRPT_CH0_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_0_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_IP1588_1_INTR0_EN) |
         (1<<VTSS_10G_GPIO_AGGR_INTRPT_GPIO_INTR_EN)),
    },
};


/* reseting the malibu */
void m10g_mode_conf(const vtss_inst_t inst, mepa_port_no_t iport, mepa_port_no_t slot_port)
{
    
    mesa_rc rc = MESA_RC_OK;
    //vtss_phy_10g_event_t       ev;
    vtss_phy_10g_mode_t oper_mode = {};
    oper_mode.oper_mode = VTSS_PHY_LAN_MODE;
    oper_mode.xfi_pol_invert = 1;
    oper_mode.polarity.host_rx = false;
    oper_mode.polarity.line_rx = false;
    oper_mode.polarity.host_tx = false;
    oper_mode.polarity.line_tx = false;
    oper_mode.is_host_wan = false;
    oper_mode.lref_for_host = false;
    oper_mode.h_clk_src.is_high_amp = true;
    oper_mode.l_clk_src.is_high_amp = true;
    oper_mode.h_media = VTSS_MEDIA_TYPE_SR;
    oper_mode.l_media = VTSS_MEDIA_TYPE_DAC;
    oper_mode.serdes_conf.l_offset_guard = true;
    oper_mode.serdes_conf.h_offset_guard = true;

    if ((rc = vtss_phy_10g_mode_set(inst, iport, &oper_mode)) != MESA_RC_OK) {
        printf("vtss_phy_10g_mode_set failed, port_no %u", iport);
    }
    vtss_gpio_10g_gpio_mode_t gpio_conf;
    const fa_malibu_gpio_port_map_t *gmap = &malibu_gpio_map[slot_port - iport]; /*have to be changed to respective slot */
     /* Configure I2c Slave pins clk,data(for SFP access on line)  */
    if ((vtss_phy_10g_gpio_mode_get(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf)) == MESA_RC_OK) {
        gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_conf.p_gpio = 2;
        //gmap->gpio_i2c_clk = 19;
        gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_CLK_OUT;
    
    if(vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_clk, &gpio_conf) != MESA_RC_OK) {
       printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
    }
        gpio_conf.mode = VTSS_10G_PHY_GPIO_OUT;
        gpio_conf.p_gpio = 3;
    //gmap->gpio_i2c_clk = 18;
        gpio_conf.in_sig = VTSS_10G_GPIO_INTR_SGNL_I2C_MSTR_DATA_OUT;
    if(vtss_phy_10g_gpio_mode_set(PHY_INST, iport, gmap->gpio_i2c_dat, &gpio_conf) != MESA_RC_OK) {
        printf("vtss_phy_10g_gpio_mode_set failed, port_no %u", iport);
    }
  }

    
}