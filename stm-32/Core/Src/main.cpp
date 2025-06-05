extern "C" {
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "eeprom.h"
#include "userled.h"
#include "LPUART.h"
#include "delay.h"
#include "gvalg.h"
#include "tab_cat.h"
#include "catalog_xyz.h"
#include "catalog_luts.h"
#include "catalog_bytestream.h"
}

// main with C linkage for correct startup symbol
extern "C" int main(void);

void SystemClock_Config(void);
double **allocate_matrix(int rows, int cols);
void free_matrix(double **mat, int rows);


extern "C" int main(void) {
	// ------------- SAMPLE STARFIELD IMAGE: -------------
	// HIP IDs used: 1, 6, 8, 13, 16
	#define n_stars_img 5
	double image_centroids[n_stars_img][3] = {
		{0.51237873, 0.02050796, 0.85851468},  // HIP 1
		{0.54280261, 0.09694802, 0.83424601},  // HIP 6
		{0.47423032, 0.11979696, 0.87221230},  // HIP 8
		{0.38942390, 0.21230918, 0.89625545},  // HIP 13
		{0.63385529, 0.38047728, 0.67339773},  // HIP 16
	};
	// Image pair angle table [indexA, indexB, angle]
	int n_image = 10;
	double **tab_image = allocate_matrix(n_image, 3);
	tab_image[0][0] = 0; tab_image[0][1] = 1; tab_image[0][2] = 0.085803;
	tab_image[1][0] = 0; tab_image[1][1] = 2; tab_image[1][2] = 0.107295;
	tab_image[2][0] = 0; tab_image[2][1] = 3; tab_image[2][2] = 0.231449;
	tab_image[3][0] = 0; tab_image[3][1] = 4; tab_image[3][2] = 0.425824;
	tab_image[4][0] = 1; tab_image[4][1] = 2; tab_image[4][2] = 0.081666;
	tab_image[5][0] = 1; tab_image[5][1] = 3; tab_image[5][2] = 0.202032;
	tab_image[6][0] = 1; tab_image[6][1] = 4; tab_image[6][2] = 0.375124;
	tab_image[7][0] = 2; tab_image[7][1] = 3; tab_image[7][2] = 0.127871;
	tab_image[8][0] = 2; tab_image[8][1] = 4; tab_image[8][2] = 0.340091;
	tab_image[9][0] = 3; tab_image[9][1] = 4; tab_image[9][2] = 0.373232;
	// ---------------------------------------------------

    // Output buffers
    int id[n_stars_img];
    double v2[n_stars_img];

    // Call algorithm
    gvalg(cat, tab_cat, tab_image, id, v2, n_image, n_stars_img);

    // Print output
    LPUART_Print("ID assignments:\n");
    for (int i = 0; i < n_stars_img; i++) {
        char buffer[128];
        int v2_rounded = (int)(v2[i] + 0.5);  // manual round instead of %.0f
        sprintf(buffer, "Image Star %d → Catalog HIP ID %d, verified %d times\n", i, id[i], v2_rounded);
        LPUART_Print(buffer);
    	}

    // Cleanup
    free_matrix(tab_image, n_image);
    return 0;
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 71;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV6;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);

    HAL_RCCEx_EnableMSIPLLMode();
}

// ------------- HELPER FUNCTIONS: -------------
// ---- allocate_matrix() ----
double **allocate_matrix(int rows, int cols) {
    double **mat = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++) {
        mat[i] = (double *)calloc(cols, sizeof(double));
    }
    return mat;
}
// ---- free_matrix() ----
void free_matrix(double **mat, int rows) {
    for (int i = 0; i < rows; i++) {
        free(mat[i]);
    }
    free(mat);
}
