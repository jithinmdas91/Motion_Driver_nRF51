/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "twi_master.h"
#include "mpu9150.h"
#include "nrf_delay.h"

/*lint ++flb "Enter library region" */

#define ADDRESS_WHO_AM_I          (0x75U) // !< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) // !<

static const uint8_t expected_who_am_i = 0x68U; // !< Expected value to get from WHO_AM_I register.
static uint8_t       m_device_address;          // !< Device address in bits [7:1]

bool mpu9150_init(uint8_t device_address)
{
    bool transfer_succeeded = true;

    m_device_address = (uint8_t)(device_address << 1);

    // Do a reset on signal paths
    uint8_t reset_value = 0x04U | 0x02U | 0x01U; // Resets gyro, accelerometer and temperature sensor signal paths.
    transfer_succeeded &= mpu9150_register_write(ADDRESS_SIGNAL_PATH_RESET, reset_value);
    
    // Read and verify product ID
    transfer_succeeded &= mpu9150_verify_product_id();
    
    if(transfer_succeeded)
    {
        uint8_t data;
                
        transfer_succeeded &= mpu9150_register_write(0x6B, 0x00);
        nrf_delay_ms(20);
                
        mpu9150_register_read(0x38, &data, 1);
        printf("gyro = %x\n", data);
    }
    return transfer_succeeded;
}

bool mpu9150_verify_product_id(void)
{
    uint8_t who_am_i;

    if (mpu9150_register_read(ADDRESS_WHO_AM_I, &who_am_i, 1))
    {
        if (who_am_i != expected_who_am_i)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

bool mpu9150_register_write(uint8_t register_address, uint8_t value)
{
    uint8_t w2_data[2];

    w2_data[0] = register_address;
    w2_data[1] = value;
    return twi_master_transfer(m_device_address, w2_data, 2, TWI_ISSUE_STOP);
}

bool mpu9150_register_read(uint8_t register_address, uint8_t * destination, uint8_t number_of_bytes)
{
    bool transfer_succeeded;
    transfer_succeeded  = twi_master_transfer(m_device_address, &register_address, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded &= twi_master_transfer(m_device_address|TWI_READ_BIT, destination, number_of_bytes, TWI_ISSUE_STOP);
    return transfer_succeeded;
}

int mpu6050_i2c_write(unsigned char slave_addr,
                     unsigned char reg_addr,
                     unsigned char length,
                     unsigned char const *data)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer(m_device_address, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded = twi_master_transfer(m_device_address, (uint8_t*)data, length, TWI_DONT_ISSUE_STOP);
    return transfer_succeeded;
}

int mpu6050_i2c_read(unsigned char slave_addr,
                     unsigned char reg_addr,
                     unsigned char length,
                     unsigned char const *data)
{
    bool transfer_succeeded;
    transfer_succeeded = twi_master_transfer(m_device_address, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
    transfer_succeeded = twi_master_transfer(m_device_address|TWI_READ_BIT, (uint8_t*)data, length, TWI_DONT_ISSUE_STOP);
    return transfer_succeeded;
}

/*lint --flb "Leave library region" */
