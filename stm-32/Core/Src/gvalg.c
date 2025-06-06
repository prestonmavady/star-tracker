// /star-tracker     rev. 05312025       preston mavady
// ----------------------------------------------------------
// Geometric Voting Algorithm (gvalgo)
//   Associates unknown image star centroids with known catalog stars
//   by voting across angular distances between star pairs.
//
//   Inputs:
//     - cat: double** [N_cat][7]
//         Catalog of known stars, where each row represents one star:
//           cat[i][0] = HIP ID (or catalog ID)        [integer as double]
//           cat[i][1] = x component of unit vector
//           cat[i][2] = y component of unit vector
//           cat[i][3] = z component of unit vector

//      ! Cat needs to be changed to rather than array of lists of 7 things [HIP_ID, RA, Dec, Mag, X, Y, Z]
//      ! It should instead be [HIP_ID, RA_ DEC]

//
//     - tab_cat: double** [N_cat_pairs][3]
//         Precomputed catalog star pair angle table:
//           tab_cat[i][0] = HIP ID of star A          [integer as double]
//           tab_cat[i][1] = HIP ID of star B          [integer as double]
//           tab_cat[i][2] = angle between A and B     [in radians]
//
//     - tab_image: double** [n_image][3]
//         Image star pair angular distances, derived from centroid vectors:
//           tab_image[i][0] = index of image centroid A   [int as double, in range 0 to n_stars_img-1]
//           tab_image[i][1] = index of image centroid B   [int as double]
//           tab_image[i][2] = angle between them (A,B)    [in radians]
//
//     - id: int* [n_stars_img]
//         Output: for each image centroid, holds the most-voted catalog HIP ID
//         (0 means unidentified)
//
//     - v2: double* [n_stars_img]
//         Output: verification scores (number of pairwise matches that confirmed ID)
//
//     - n_image: int
//         Number of image star pairs (rows in tab_image)
//
//     - n_stars_img: int
//         Number of image centroids (distinct star points in the image)
//
// ----------------------------------------------------------
//
//   Output:
//     - id[i] = HIP ID of the catalog star identified as matching image centroid i
//     - v2[i] = number of verified angle matches that support this ID (higher = more reliable)
//
//   Algorithm Summary:
//     > For each image pair (i,j), find catalog star pairs with similar angles
//     > "Vote" for all stars in those catalog pairs to be possible matches
//     > Each image centroid accumulates votes from many pairs
//     > Take the mode (most frequent vote) as the ID guess
//     > Verify IDs by checking if their unit vectors match known catalog angles
//
// ----------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "LPUART.h"
#include "catalog_luts.h"
#include "catalog_bytestream.h"

uint8_t setmode(int nums[], int numsize);

void gvalg(const double cat[][4], const double tab_cat[][3], double **tab_image,
           int *id, double *v2, int n_image, int n_stars_img) {


    // ---------------- MEMORY ALLOCATION ----------------
    // allocate k_n - (used in future loops) counts how many
    // catalog pairs have been voted for a specific star in the image
    int *k_n = (int *)calloc(n_stars_img, sizeof(int));

    // allocate v - an array for storing matches for each centroid.
    // Each row v[i] holds a list of voted catalog star IDs
    int **v = (int **)calloc(n_stars_img, sizeof(int *));
    for (int i = 0; i < n_stars_img; i++) {
        v[i] = (int *)calloc(512, sizeof(int));  // Enough space for 256 pairs (2 votes per pair)
    }


    // ---------------- FIRST ROUND ----------------
    // for each centroid pair from the image (each entry in tab_image[i][3]
    for (int i = 0; i < n_image; i++) {

    	// store the angle of the current pair (in millidegrees
    	int angle = tab_image[i][2];
        // find the starting address of the bin (milidegrees/100 - i.e. 84000/100 = 840)
    	int bin_index = (int)(angle/100);
    	int bin_addr = bin_angle_to_address_lut[bin_index];
        // starting index to add a vote to (location of first pair in bin)
        int index_min = (bin_addr + 1);
        // stopping index to add a vote to (location of last pair in bin)
        int index_max = (bin_addr + catalog_bytestream[bin_addr]);



        // --- DEBUG 1 DEBUG 1 DEBUG 1 DEBUG 1 ---
        char dbg[256];

        // Print image pair and angle
        snprintf(dbg, sizeof(dbg), "\n[DEBUG] Image Pair %d: centroid (%d, %d), angle = %d\r\n",
                 i, (int)tab_image[i][0], (int)tab_image[i][1], angle);
        LPUART_Print(dbg);

        // Print bin info
        snprintf(dbg, sizeof(dbg), "         → bin_index = %d, bin_addr = %d\r\n", bin_index, bin_addr);
        LPUART_Print(dbg);
        snprintf(dbg, sizeof(dbg), "         → index_min = %d, index_max = %d\r\n", index_min, index_max);
        LPUART_Print(dbg);
        // ------------------------------------


        for (int j = index_min; j <= index_max + 4; j++) {// iterate through all pairs in bin

        	int idx0 = (int)tab_image[i][0];
        	int idx1 = (int)tab_image[i][1];

        	// Vote for star A
        	v[idx0][2 * k_n[idx0] + 0] = (int)catalog_bytestream[j];
        	v[idx0][2 * k_n[idx0] + 1] = (int)catalog_bytestream[j+1];

        	// Vote for star B
        	v[idx1][2 * k_n[idx1] + 0] = (int)catalog_bytestream[j];
        	v[idx1][2 * k_n[idx1] + 1] = (int)catalog_bytestream[j+1];

            // --- DEBUG 2 DEBUG 2 DEBUG 2 DEBUG 2 ---
            // Vote print for tab_image[i][0]
            snprintf(dbg, sizeof(dbg), "         → VOTE[%d] += (%d, %d)\r\n",
            (int)tab_image[i][0], v[idx1][2 * k_n[idx1] + 0], v[idx0][2 * k_n[idx0] + 1]);
            LPUART_Print(dbg);

            // Vote print for tab_image[i][1]
            snprintf(dbg, sizeof(dbg), "         → VOTE[%d] += (%d, %d)\r\n",
            (int)tab_image[i][1], v[idx1][2 * k_n[idx1] + 0], v[idx1][2 * k_n[idx1] + 1]);
            LPUART_Print(dbg);
            delay_us(200000);
            // ------------------------------------


        	// Now increment vote counters
        	k_n[idx0]++;
        	k_n[idx1]++;
        	j+=1;
        }
    }

    LPUART_Print(" \r\n");
    for (int i = 0; i < n_stars_img; i++) {
        char buf[128];
        sprintf(buf, "Centroid %d received %d votes\r\n", i, k_n[i] * 2);
        LPUART_Print(buf);
    }


    // -*-*-----*-*- count votes -*-*------*-*-
    for (int i = 0; i < n_stars_img; i++) {
    	int vote_count = k_n[i] * 2;  // Each pair adds 2 votes
    	id[i] = setmode(v[i], vote_count);

        // Fallback in case of invalid match
        if (id[i] < 0) id[i] = 0;
    }

    LPUART_Print(" \r\n");
    for (int i = 0; i < n_stars_img; i++) {
        char buf[128];
        sprintf(buf, "Centroid %d assigned HIP ID %d\r\n", i, id[i]);
        LPUART_Print(buf);
    }


    // ---------------- SECOND ROUND ----------------
    // Now that stars have been matched, calculate real pairwise
    // angles from catalog to confirm that matches are correct.
    for (int i = 0; i < n_image; i++) {
    	if (tab_image[i][0] >= 0 && tab_image[i][1] >= 0) {
            int a = id[(int)tab_image[i][0]];
            int b = id[(int)tab_image[i][1]];

            // Check that both centroids were successfully ID'd
            if (a != 0 && b != 0) {
                // Calculate catalog angle between identified stars
            	int a_idx = -1, b_idx = -1;
            	for (int j = 0; j < 256; j++) {
            	    if ((int)cat[j][0] == a) a_idx = j;
            	    if ((int)cat[j][0] == b) b_idx = j;
            	}

            	if (a_idx >= 0 && b_idx >= 0) {
            	    double d_cat = acos(cat[a_idx][1] * cat[b_idx][1] +
            	                        cat[a_idx][2] * cat[b_idx][2] +
            	                        cat[a_idx][3] * cat[b_idx][3]);

            	    double d_image = tab_image[i][2];

            	    if (fabs(d_cat - d_image) < 1) {
            	        v2[(int)tab_image[i][0]]++;
            	        v2[(int)tab_image[i][1]]++;
            	    }
            	}

            }
        }
    }

    // --- Free dynamically allocated memory ---
    free(k_n);
    for (int i = 0; i < n_stars_img; i++)
    {
        free(v[i]);
    }
    free(v);
}

uint8_t setmode(int nums[], int numsize){
	int mode = 0;
    int counting = 0;
    int maxcount = 0;

    for (int i = 0; i < numsize; i++) {
        for (int x = 0; x < numsize; x++) {
            if (nums[i] == nums[x]) {
                counting++;
            }
        }
        if (counting > maxcount) {
            mode = nums[i];
            maxcount = counting;
        }
        counting = 0;
    }
    return mode;
}
