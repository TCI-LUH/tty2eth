#include "ksz9563r.h"
#include "stm32h7xx_hal.h"

#define KSZ9563R_SW_RESET_TO ((uint32_t)500U)
#define KSZ9563R_BUS_WAIT ((uint32_t)100U)
#define KSZ9563R_INIT_TO ((uint32_t)2000U)

int32_t KSZ9563R_RegisterBusIO(ksz9563r_t *pObj, ksz9563r_IOCtx_t *ioctx)
{
    if (!pObj || !ioctx->ReadReg || !ioctx->WriteReg || !ioctx->GetTick)
    {
        return KSZ9563R_STATUS_ERROR;
    }

    pObj->IO = *ioctx;
    // pObj->IO.Init = ioctx->Init;
    // pObj->IO.DeInit = ioctx->DeInit;
    // pObj->IO.ReadReg = ioctx->ReadReg;
    // pObj->IO.WriteReg = ioctx->WriteReg;
    // pObj->IO.ReadReg16 = ioctx->ReadReg16;
    // pObj->IO.WriteReg16 = ioctx->WriteReg16;
    // pObj->IO.GetTick = ioctx->GetTick;

    return KSZ9563R_STATUS_OK;
}

uint32_t KSZ9563R_SetPortStateForwarding(ksz9563r_t *pObj, int port)
{
    uint32_t value = KSZ9563R_STATUS_ERROR;

    if(port < KSZ9563_PORT1 || port > KSZ9563_PORT2)
        return KSZ9563R_STATUS_ERROR;

    uint32_t status = pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_PORTn_MSTP_STATE(port), &value);
    if (status != HAL_OK)
        return KSZ9563R_STATUS_READ_ERROR;


    value |= KSZ9563_PORTn_MSTP_STATE_TRANSMIT_EN;
    value |= KSZ9563_PORTn_MSTP_STATE_RECEIVE_EN;
    value &= ~KSZ9563_PORTn_MSTP_STATE_LEARNING_DIS;

    status = pObj->IO.WriteReg(pObj->DevAddr, KSZ9563_PORTn_MSTP_STATE(port), value);
    if (status != HAL_OK)
        return KSZ9563R_STATUS_WRITE_ERROR;
    return KSZ9563R_STATUS_OK;
}

/**
 * @brief  Initialize the KSZ9563R and configure the needed hardware resources
 * @param  pObj: device object KSZ9563R_Object_t.
 * @retval KSZ9563R_STATUS_OK  if OK
 *         KSZ9563R_STATUS_ADDRESS_ERROR if cannot find device address
 *         KSZ9563R_STATUS_READ_ERROR if connot read register
 *         KSZ9563R_STATUS_WRITE_ERROR if connot write to register
 *         KSZ9563R_STATUS_RESET_TIMEOUT if cannot perform a software reset
 */
int32_t KSZ9563R_Init(ksz9563r_t *pObj)
{
    uint32_t tickstart = 0, regvalue = 0;
    int32_t status = KSZ9563R_STATUS_OK;

    if (pObj->Is_Initialized == 0)
    {
        if (pObj->IO.Init != 0)
        {
            /* GPIO and Clocks initialization */
            pObj->IO.Init();
        }

        pObj->DevAddr = 0x5F;

        status = KSZ9563R_STATUS_ADDRESS_ERROR;
        for(int i = 0; i < 30; i++)
        {
            while ((pObj->IO.GetTick() - tickstart) <= KSZ9563R_BUS_WAIT)
            {
            }

            if (pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_CHIP_ID1, &regvalue) < 0)
                continue;

            if(regvalue != KSZ9563_CHIP_ID1_DEFAULT)
                continue;

            status = KSZ9563R_STATUS_OK;
            break;
        }
        /* Get the device address from special mode register */
       

        if (status != KSZ9563R_STATUS_OK)
        {
            status = KSZ9563R_STATUS_ADDRESS_ERROR;
        }

        /* if device address is matched */
        if (status == KSZ9563R_STATUS_OK)
        {
            /* set a software reset  */
            if (pObj->IO.WriteReg(pObj->DevAddr, KSZ9563_SWITCH_OP,  KSZ9563_SWITCH_OP_SOFT_HARD_RESET) >= 0)
            {
                /* get software reset status */
                if (pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_SWITCH_OP, &regvalue) >= 0)
                {
                    tickstart = pObj->IO.GetTick();

                    /* wait until software reset is done or timeout occured  */
                    while (regvalue & KSZ9563_SWITCH_OP_SOFT_HARD_RESET)
                    {
                        if ((pObj->IO.GetTick() - tickstart) <= KSZ9563R_SW_RESET_TO)
                        {
                            if (pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_SWITCH_OP, &regvalue) < 0)
                            {
                                status = KSZ9563R_STATUS_READ_ERROR;
                                break;
                            }
                        }
                        else
                        {
                            status = KSZ9563R_STATUS_RESET_TIMEOUT;
                            break;
                        }
                    }
                }
                else
                {
                    status = KSZ9563R_STATUS_READ_ERROR;
                }
            }
            else
            {
                status = KSZ9563R_STATUS_WRITE_ERROR;
            }
        }
    }

    if (status == KSZ9563R_STATUS_OK)
    {
        for(int port = KSZ9563_PORT1; port <= KSZ9563_PORT2; port++)
        {
            status = KSZ9563R_SetPortStateForwarding(pObj, port);
            if(status != KSZ9563R_STATUS_OK)
                return status;
        }

         uint32_t value = KSZ9563R_STATUS_ERROR;


        {
            uint32_t status = 0;
            uint32_t ctlReg =  KSZ9563_BMCR_DUPLEX_MODE | KSZ9563_BMCR_SPEED_SEL_LSB;

            status = pObj->IO.WriteReg16(pObj->DevAddr, KSZ9563_PORTn_ETH_PHY_REG(KSZ9563_PORT1, 0), ctlReg);
            if (status != HAL_OK)
                return KSZ9563R_STATUS_WRITE_ERROR;

            do
            {
                status = pObj->IO.ReadReg16(pObj->DevAddr, KSZ9563_PORTn_ETH_PHY_REG(KSZ9563_PORT2, 0), &ctlReg);
                if (status != HAL_OK)
                    return KSZ9563R_STATUS_READ_ERROR;
            }while(ctlReg & KSZ9563_BMCR_RESET);
        }

        tickstart = pObj->IO.GetTick();

        /* Wait for 2s to perform initialization */
        while ((pObj->IO.GetTick() - tickstart) <= KSZ9563R_INIT_TO)
        {
        }
        pObj->Is_Initialized = 1;
    }

    return status;
}

/**
 * @brief  De-Initialize the KSZ9563R and it's hardware resources
 * @param  pObj: device object KSZ9563R_Object_t.
 * @retval None
 */
int32_t KSZ9563R_DeInit(ksz9563r_t *pObj)
{
    if (pObj->Is_Initialized)
    {
        if (pObj->IO.DeInit != 0)
        {
            if (pObj->IO.DeInit() < 0)
            {
                return KSZ9563R_STATUS_ERROR;
            }
        }

        pObj->Is_Initialized = 0;
    }

    return KSZ9563R_STATUS_OK;
}

int32_t KSZ9563R_GetLinkSpeed(ksz9563r_t *pObj)
{
    uint32_t value = 0;
    uint32_t linkSpeed = KSZ9563R_STATUS_ERROR;
    int32_t status = 0;

    // Read port 3 XMII control 1 register
    status = pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_PORT3_XMII_CTRL1, &value);
    if (status != HAL_OK)
        return KSZ9563R_STATUS_READ_ERROR;
    // Retrieve host interface type
    uint8_t type = value & KSZ9563_PORTn_XMII_CTRL1_IF_TYPE;

    // Gigabit interface?
    if (type == KSZ9563_PORTn_XMII_CTRL1_IF_TYPE_RGMII &&
        (value & KSZ9563_PORTn_XMII_CTRL1_SPEED_1000) != 0)
    {
        // 1000 Mb/s mode
        linkSpeed = KSZ9563R_STATUS_1000MBITS_FULLDUPLEX;
    }
    else
    {
        // Read port 3 XMII control 0 register
        status = pObj->IO.ReadReg(pObj->DevAddr, KSZ9563_PORT3_XMII_CTRL0, &value);
        if (status != HAL_OK)
            return KSZ9563R_STATUS_READ_ERROR;

        // Retrieve host interface speed
        if ((value & KSZ9563_PORTn_XMII_CTRL0_SPEED_10_100) != 0)
        {
            // 100 Mb/s mode
            linkSpeed = KSZ9563R_STATUS_100MBITS_FULLDUPLEX;
        }
        else
        {
            // 10 Mb/s mode
            linkSpeed = KSZ9563R_STATUS_10MBITS_FULLDUPLEX;
        }
    }
    return linkSpeed;
}

int32_t KSZ9563R_GetPortLinkState(ksz9563r_t *pObj, uint8_t port)
{
    uint32_t value = 0;
    int32_t linkState;
    int32_t status = 0;

    // Check port number
    if (port >= KSZ9563_PORT1 && port <= KSZ9563_PORT2)
    {
        // Any link failure condition is latched in the BMSR register. Reading
        // the register twice will always return the actual link status
        const uint32_t reg = KSZ9563_PORTn_ETH_PHY_REG(port, KSZ9563_BMSR);
        status = pObj->IO.ReadReg(pObj->DevAddr, reg, &value);
        if (status != HAL_OK)
            return KSZ9563R_STATUS_READ_ERROR;
        status = pObj->IO.ReadReg(pObj->DevAddr, reg, &value);
        if (status != HAL_OK)
            return KSZ9563R_STATUS_READ_ERROR;

        // Retrieve current link state
        linkState = (value & KSZ9563_BMSR_LINK_STATUS) ? KSZ9563R_STATUS_OK : KSZ9563R_STATUS_LINK_DOWN;
    }
    else
    {
        // The specified port number is not valid
        linkState = KSZ9563R_STATUS_ERROR;
    }

    // Return link status
    return KSZ9563R_STATUS_OK;
}

int32_t KSZ9563R_GetLinkState(ksz9563r_t *pObj)
{
    uint32_t linkstatus = KSZ9563R_GetPortLinkState(pObj, KSZ9563_PORT2);

    /* Read Status register  */
    if (linkstatus < 0)
    {
        return linkstatus;
    }

    if (linkstatus == KSZ9563R_STATUS_LINK_DOWN)
    {
        /* Return Link Down status */
        return KSZ9563R_STATUS_LINK_DOWN;
    }

    return KSZ9563R_GetLinkSpeed(pObj);
}
