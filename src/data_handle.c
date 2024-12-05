#include "data_handle.h"
#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// needed to free char node
void free_charnode(CharNode * n)
{
    CharNode * temp; 

    while(n != NULL)
    {
        temp = n;
        n    = n -> next;
        free(temp);
    }
}

/*
    Helper function to create nodes for the features
*/
static CharNode * create_charnode(void)
{
    CharNode * new  = (CharNode *) malloc(sizeof(CharNode));
    new -> val  = "";
    new -> next = NULL;

    return new;
}

/*
    Creates a dataframe for a given csv file, and returns the dimensions
*/
Dimension get_frame(FILE * fp, double *** data, CharNode * features)
{
    get_features(fp, features);
    Dimension dim = get_rows_cols(fp);
    construct_frame(dim, data);
    fill_frame(fp, dim, * data);

    return dim;
}

/*
    Returns a linked list containing the features of the dataset
*/
void get_features(FILE * fp, CharNode * features)
{
    CharNode * curr = features;
    char   line[LINE_BUFF];
    char * tok;
    
    fgets(line, LINE_BUFF, fp);
    fseek(fp, 0, SEEK_SET);
    
    tok = strtok(line, ",");
    while(tok != NULL)
    {
        curr -> val  = strdup(tok);
        curr -> next = create_charnode();
        curr = curr -> next;

        tok = strtok(NULL, ",");
    }
}

/*
    Returns the number of rows/columns of a csv file 
*/
Dimension get_rows_cols(FILE *fp)
{
    Dimension dim;
    dim.row = 0; // rows
    dim.col = 0; // columns

    char   line[LINE_BUFF]; // line buffer
    char * tok;

    bool first_run = true;
    while(fgets(line, LINE_BUFF, fp) != NULL)
    {
        if(first_run)
        {
            tok = strtok(line, ",");     // create token separating ","
            while(tok != NULL)
            {
                dim.col++;               // increment columns (only on first run)
                tok = strtok(NULL, ","); // repeat until all columns found
            }
            first_run = false;
        }
        else dim.row++;                  // increment rows (ignoring features row)
    }

    free(tok);              // free pointed value
    fseek(fp, 0, SEEK_SET); // return to top of file

    return dim;
}

/*
    Constructs the dataframe for the given size
*/
void construct_frame(Dimension dim, double *** data)
{
    int rows = dim.row; // grab rows
    int cols = dim.col; // grab cols

    * data = (double **) malloc(rows * sizeof(double *));     // malloc rows of dataframe
    for(int i = 0; i < rows; i++)
    {
        (* data)[i] = (double *) malloc(cols * sizeof(double)); // malloc columns of dataframe
    }
}

/*
    Fills the frame with the data from the file, once it's already been constructed
*/
void fill_frame(FILE * fp, Dimension dim, double ** data)
{
    int rows = dim.row;     // number of rows
    int cols = dim.col;     // number of columns
    
    char   line[LINE_BUFF]; // line buffer
    char * tok;             // string token

    fgets(line, LINE_BUFF, fp); // skip features row

    for(int i = 0; i < rows; i++)     // iterate through the rows
    {
        fgets(line, LINE_BUFF, fp);   // grab our current line
        tok = strtok(line, ",");      // tokenize line

        for(int j = 0; j < cols; j++) // iterate through the columns
        {
            data[i][j] = strtod(tok, NULL);   // save to current section
            
            tok = strtok(NULL, ",");  // retokenize
        }
    }
}

void print_frame(Dimension dim, double ** data, CharNode * features)
{
    printf("   |");
    CharNode * curr = features;
    while(curr -> next != NULL)
    {
        printf(" %s", curr -> val);
        curr = curr -> next;
    }

    printf("   |\n");

    for (int i = 0; i < dim.row; i++) {
        printf("%3d|", i);
        for (int j = 0; j < dim.col; j++) {
            printf(" %0.1lf", data[i][j]);
        }
        printf("\n");
    }
}

void free_frame(double ** data, Dimension dim)
{
    for (int i = 0; i < dim.row; i++) 
    {
        free(data[i]);
    }
    free(data);
}
