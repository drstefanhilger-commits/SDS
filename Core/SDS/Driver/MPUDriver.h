/*
 * MPU_Init.h
 *
 *  Created on: Aug 13, 2026
 *      Author: 310004
 */
#pragma once

static void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* MPU deaktivieren, bevor wir konfigurieren */
    HAL_MPU_Disable();

    /*----------------------------------------------------------*/
    /* Region 0: AXI-SRAM (0x20010000) als uncached RAM        */
    /*----------------------------------------------------------*/
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x20010000;                 // AXI-SRAM Start
    MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;      // je nach MCU ggf 512KB
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = 0;
    MPU_InitStruct.IsCacheable      = 0;                          // <<< WICHTIG: uncached
    MPU_InitStruct.IsShareable      = 0;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = 0;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /*----------------------------------------------------------*/
    /* Region 1: SDRAM (0xC0000000) als uncached Framebuffer   */
    /*----------------------------------------------------------*/
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0xC0000000;                 // SDRAM Start
    MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = 0;
    MPU_InitStruct.IsCacheable      = 0;                          // <<< WICHTIG: uncached für LTDC
    MPU_InitStruct.IsShareable      = 0;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = 0;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /*----------------------------------------------------------*/
    /* MPU wieder aktivieren (privileged default)              */
    /*----------------------------------------------------------*/
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
