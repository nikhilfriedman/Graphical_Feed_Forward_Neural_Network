#ifndef __DATA_HANDLE_H__
#define __DATA_HANDLE_H__

#include <stdio.h>
#include "definitions.h"

void      free_charnode  (CharNode * n);
Dimension get_frame      (FILE * fp, double *** data, CharNode * features);
void      get_features   (FILE * fp, CharNode * features);
Dimension get_rows_cols  (FILE *fp);
void      construct_frame(Dimension dim, double *** data);
void      fill_frame     (FILE * fp, Dimension dim, double ** data);
void      print_frame    (Dimension dim, double ** data, CharNode * features);
void      free_frame     (double ** data, Dimension dim);

#endif