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

void SystemClock_Config(void);
double **allocate_matrix(int rows, int cols);
void free_matrix(double **mat, int rows);

int main(void) {
	 HAL_Init();
	 SystemClock_Config();
	 LPUART_Config();
	 Userled_init();
	 Delay_init();

    // >> Clear Screen
	LPUART_Send_ESC_Seq("2J");
    LPUART_Send_ESC_Seq("H");
    // >> -----------
    LPUART_Print("Starting GValg... ...\r\n");
	// ------------- SAMPLE STARFIELD IMAGE: -------------
	// HIP IDs used: 1, 6, 8, 13, 16
	#define n_stars_img 4
	// We would take in a list of centroids, calculate their body-frame
	// unit vector, and then create this image_centrodis[] list
	// double image_centroids[n_stars_img][3] = {
	//	{0.51237873, 0.02050796, 0.85851468},  // CENT 0 (HIP 1 = 746)
	//	{0.54280261, 0.09694802, 0.83424601},  // CENT 1 (HIP 6 = 3179)
	//	{0.47423032, 0.11979696, 0.87221230},  // CENT 2 (HIP 8 = 4427)
	//	{0.38942390, 0.21230918, 0.89625545},  // CENT 3 (HIP 13 = 8886)
	// };
	// Image pair angle table [indexA, indexB, angle_millideg]
	#define n_image 6
	double **tab_image = allocate_matrix(n_image, 3);

	tab_image[0][0] = 0; tab_image[0][1] = 1; tab_image[0][2] = 4916;   // 1 and 6
	tab_image[1][0] = 0; tab_image[1][1] = 2; tab_image[1][2] = 6147;   // 1 and 8
	tab_image[2][0] = 0; tab_image[2][1] = 3; tab_image[2][2] = 13261;  // 1 and 13
	tab_image[3][0] = 1; tab_image[3][1] = 2; tab_image[3][2] = 4679;   // 6 and 8
	tab_image[4][0] = 1; tab_image[4][1] = 3; tab_image[4][2] = 11575;  // 6 and 13
	tab_image[5][0] = 2; tab_image[5][1] = 3; tab_image[5][2] = 7326;   // 8 and 13


//	tab_image[0][0] = 0; tab_image[0][1] = 1; tab_image[0][2] = 4916; // 1 and 6
//	// bin 49: 1,6 + 40,44 + 160,166 + 161,167
//	tab_image[1][0] = 0; tab_image[1][1] = 2; tab_image[1][2] = 6147; // 1 and 8
//	// bin 61: 1,8, 125,129 + 155,161 + 213,218...
//	tab_image[2][0] = 0; tab_image[2][1] = 3; tab_image[2][2] = 13261; // 1 and 13
//	// bin 132: 1,13 + 29,36 + 39,46...
//	tab_image[3][0] = 1; tab_image[4][1] = 2; tab_image[4][2] = 4679; // 6 and 8
//	// bin 46: 6,8 + 160,167 + 213,215 + 213,216
//	tab_image[4][0] = 1; tab_image[5][1] = 3; tab_image[5][2] = 11575; // 6 and 13
//	tab_image[5][0] = 2; tab_image[7][1] = 3; tab_image[7][2] = 7326; // 8 and 13
    LPUART_Print("Successfully Created Fake Starfield... ...\r\n");
	// ---------------------------------------------------

    // Output buffers
    int id[n_stars_img];
    double v2[n_stars_img];
    for (int i = 0; i < n_stars_img; i++) {
        v2[i] = 0;
    }

    // Call algorithm
    gvalg(cat, tab_cat, tab_image, id, v2, n_image, n_stars_img);
    LPUART_Print("Successfully Ran GValg... ...\r\n\r\n");

    // Print output
    LPUART_Print(":: STAR MATCHES ::\r\n");
    for (int i = 0; i < n_stars_img; i++) {
        char buffer[128];
        int v2_rounded = (int)(v2[i] + 0.5);  // manual round instead of %.0f
        sprintf(buffer, "Image Star %d → Catalog HIP ID %d, verified %d times\r\n", i, id[i], v2_rounded);
        LPUART_Print(buffer);
    	}

    // Cleanup
    free_matrix(tab_image, n_image);

    // Blink LED if Done
    for (int i = 0; i < 100; i++){
    	userled_set(1);
    	delay_us(20000);
    	userled_set(0);
    	delay_us(20000);
    }

    return 0;

}

void SystemClock_Config(void)
{
	 RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	 RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	 /** Configure the main internal regulator output voltage */

	 /** Initializes the RCC Oscillators according to the specified parameters
	 in the RCC_OscInitTypeDef structure. */
	 RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	 RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	 RCC_OscInitStruct.MSICalibrationValue = 0;
	 RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	 RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

	 /** Initializes the CPU, AHB and APB buses clocks */
	 RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                             |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	 RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	 RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	 RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	 RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
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
