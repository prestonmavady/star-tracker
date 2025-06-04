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
//           cat[i][1] = (unused or RA)                [optional]
//           cat[i][2] = (unused or Dec)               [optional]
//           cat[i][3] = (unused or magnitude)         [optional]
//           cat[i][4] = x component of unit vector
//           cat[i][5] = y component of unit vector
//           cat[i][6] = z component of unit vector

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
//     - loc_err: double
//         Tolerance threshold (±) for angle matching [radians]
//         Controls how closely image angles must match catalog angles.
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

// gvalg - main geometric voting function
void gvalg(double **cat, double **tab_cat, double **tab_image, double loc_err,
            int *id, double *v2, int n_image, int n_stars_img) {
    
    // Allocate an array to keep track of how many votes each centroid receives
    int *k_n = (int *)calloc(n_stars_img, sizeof(int));

    // Allocate a 2D array for storing votes for each centroid.
    // Each row v[i] holds a flat list of voted catalog star IDs
    int **v = (int **)calloc(n_stars_img, sizeof(int *));
    for (int i = 0; i < n_stars_img; i++) {
        v[i] = (int *)calloc(512, sizeof(int));  // Enough space for 256 pairs (2 votes per pair)
    }

    // --- First round: Vote based on angular matches ---
    for (int i = 0; i < n_image; i++) {
        // Find all catalog pairs where angle ≈ image angle ± loc_err
        int index_min = bsearch(tab_cat, tab_image[i][2] - loc_err);
        int index_max = bsearch(tab_cat, tab_image[i][2] + loc_err);

        // For each catalog pair match, vote for both catalog stars
        for (int j = index_min; j <= index_max; j++) {
            // tab_image[i][0] and tab_image[i][1] are image centroid indices forming a pair

            // Vote for catalog stars [0] and [1] to be IDs of image centroids
            v[(int)tab_image[i][0]][2 * k_n[(int)tab_image[i][0]] + 1] = (int)tab_cat[j][0];
            v[(int)tab_image[i][0]][2 * k_n[(int)tab_image[i][0]] + 2] = (int)tab_cat[j][1];
            k_n[(int)tab_image[i][0]]++;

            v[(int)tab_image[i][1]][2 * k_n[(int)tab_image[i][1]] + 1] = (int)tab_cat[j][0];
            v[(int)tab_image[i][1]][2 * k_n[(int)tab_image[i][1]] + 2] = (int)tab_cat[j][1];
            k_n[(int)tab_image[i][1]]++;
        }
    }

    // --- Identify the most-voted ID for each image centroid ---
    for (int i = 0; i < n_stars_img; i++) {
        int first_zero = 0;
        // Count how many votes were cast (find the first zero as end marker)
        while (v[i][first_zero] != 0) {
            first_zero++;
        }

        // Choose the most frequent catalog ID as the identified star
        id[i] = mode(v[i], first_zero);

        // Fallback in case of invalid match
        if (isnan(id[i])) {
            id[i] = 0;
        }
    }

    // --- Second round: Verify that chosen star IDs are consistent ---
    for (int i = 0; i < n_image; i++) {
        if (tab_image[i][0] != 0 && tab_image[i][1] != 0) {
            int a = id[(int)tab_image[i][0]];
            int b = id[(int)tab_image[i][1]];

            // Check that both centroids were successfully ID'd
            if (a != 0 && b != 0) {
                // Calculate catalog angle between identified stars
                double d_cat = acos(cat[a][4] * cat[b][4] +
                                    cat[a][5] * cat[b][5] +
                                    cat[a][6] * cat[b][6]);

                double d_image = tab_image[i][2];

                // If the real catalog angle is close to the image angle, increase confidence score
                if (d_image - loc_err < d_cat && d_cat < d_image + loc_err) {
                    v2[(int)tab_image[i][0]]++;
                    v2[(int)tab_image[i][1]]++;
                }
            }
        }
    }

    // --- Free dynamically allocated memory ---
    free(k_n);
    for (int i = 0; i < n_stars_img; i++) {
        free(v[i]);
    }
    free(v);
}
