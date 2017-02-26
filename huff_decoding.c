/* 
Author 		: Ravisutha Sakrepatna Srinivasamurthy
Assignment	: Huffman Codec (Decompression)
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ERR -1
#define SUCCESS 0
#define NUM_SYM 256

typedef struct 
{
		int freq;
		char *codes;
		unsigned char ch;
}freq_table;

typedef struct tree
{
		struct tree *left, *right;
		int freq;
}trees;

/* Prototypes goes here */
void get_frequencies (FILE *fp, freq_table *table);
void fill_data (freq_table *table);
void print_frequency (trees **data);
int my_cmp2 (const void *data1, const void *data2);
int build_tree (trees **root, freq_table *table);
trees *connect_node (trees *symbol1, trees *symbol2);
void get_symbol (trees **symbol, trees **roots, freq_table *table);
int huff_decompress (FILE *fp_read, trees **root, char *filename, unsigned int final_info);
void get_codes (char **codes, unsigned char ch, trees **root);
void strrev (char *str);
void printbits (char temp);
long int size_of_file (FILE *fp);
void write_table (FILE *fp, trees **root);
int code_cmp(char *codes, trees **root, freq_table *code_table);

int main (int argc, char *argv[])
{
		/* Declarations */
		FILE *fp_read, 	*fp_write;
		freq_table 		*table = (freq_table *)calloc (NUM_SYM, sizeof (freq_table));			/* Create table */
		char      		*filename;
		long int 		final_info;
		trees 			**root = calloc (256, sizeof (trees *));

		/* Check for correct command line inputs */
		if (argc < 2)
		{
				printf ("Please provide command line argument\n");
				printf ("./huff_coding [filename]\n");

				return ERR;
		}

		/* Get filename */
		filename = malloc (strlen (argv[1]) + 1);
		strcpy (filename, argv[1]);

		/* Open file */
		if ((fp_read = fopen (filename, "rb")) == NULL)
		{
				printf ("%s:\n", filename);
				perror ("File op:");
				return ERR;
		}

		/* Fill data */
		fill_data (table);

		fread (&final_info, 1, sizeof (long int), fp_read);

		/* Get frequencies of symbols*/
		get_frequencies (fp_read, table);
		printf ("Successful read %s file and frequncy table has been constructed...\n", filename);
		sleep (1);

		/* Buid the tree using table */
		build_tree (root, table);
		printf ("Tree building complete...\n");
		sleep (1);

		/* Decompress the given file */
		huff_decompress (fp_read, root, "decompressed", final_info);
		printf ("Decompression successful...\n");
		sleep (1);

		fcloseall ();
		return SUCCESS;
}

void fill_data (freq_table *table)
{
		/* Declaration */
		int i;

		/* Fill data */
		for (i = 0; i < 256; i++)
		{
				(table + i) -> ch = (char)i;
		}

		return;
}

void get_frequencies (FILE *fp, freq_table *table)
{
		/* Declarations */
		int data, i;

		/* Calculate the frequency of occurance */
		for (i = 0; i < 256; i++)
		{
				fread (&data, 1, sizeof (int), fp);
				table[i].freq = data;
		}

		return;
}

int build_tree (trees **root, freq_table *table)
{
		/* Declarations */
		int i;
		int (*func_ptr)(const void *, const void*) = &my_cmp2;

		trees **symbol = (trees **)calloc (256, sizeof(trees *));

		get_symbol (symbol, root, table);

		qsort(symbol, NUM_SYM, sizeof (trees *), func_ptr);


		for (i = 0; i < 255; i++)
		{
				symbol[i+1] = connect_node (symbol[i], symbol[i + 1]);

				qsort(symbol, NUM_SYM, sizeof (trees *), func_ptr);
		}
}

void get_symbol (trees **symbol, trees **root, freq_table *table)
{
		int i;

		for (i = 0; i < NUM_SYM; i++)
		{
				root[i] = calloc (1, sizeof (trees));
				symbol[i] = root[i];
				root[i]->freq = table[i].freq;
		}
}

trees *connect_node (trees *symbol1, trees *symbol2)
{
		trees *new = calloc (1, sizeof (trees));

		symbol1->left = NULL;
		symbol1->right = new;

		symbol2->left = new;
		symbol2->right = NULL;

		new->freq = symbol1->freq + symbol2->freq;

		return new;
}

int huff_decompress (FILE *fp_read, trees **root, char *filename, unsigned int final_info)
{
		/* Declarations */
		FILE *fp_write = fopen (filename, "wb");
		unsigned char num = (char)0;
		unsigned int ch;
		char *codes;
		freq_table *code_table = calloc (256, sizeof (freq_table));
		int i, j, limit, k, out;

		/* Open the compression file */
		if (fp_write == NULL)
		{
				printf ("Filename : %s\n", filename);
				perror ("Opening");
		}
		
		for (ch = 0; ch <= 255; ch++)
		{
				if (root[ch] -> freq != 0)
				{
						get_codes (&codes, (char)ch, root);
						strrev (codes);
						code_table[ch].codes = calloc (1, strlen (codes) + 1);
						strcpy (code_table[ch].codes, codes);
						free (codes);
				}
		}
		
		limit = 10;
		codes = calloc (1, limit);
		j = 0;
		k = 0;

		while (fread ((char *)&ch, 1, 1, fp_read) > 0)
		{
				if ((k % 10000) == 0)
				{
						printf ("k = %d\n", k);
				}

				if (k == final_info)
				{
						break;
				}

				for (i = 0; i < 8; i++, j++)
				{
						if (j == limit)
						{
								limit = limit + 8;
								codes = realloc (codes, limit);
						}

						if ((ch & (1 << (7 - i))) > 0)
						{
								strcat (codes, "1");
						}

						else
						{
								strcat (codes, "0");
						}

						if ((out = code_cmp(codes, root, code_table)) > 0)
						{
								limit = 10;
								j = 0;
								out--;
								fwrite (&out, 1, 1, fp_write);
								free (codes);
								codes = calloc (1, limit);
								k++;
						}
				}
		}

		return SUCCESS;
}

void get_codes (char **codes, unsigned char ch, trees **root)
{
		/* Declaration goes here */
		int i = 0, limit = 10;
		trees *traverse = root[ch];
		*codes = calloc (limit, 1);

		while (!(traverse->left == NULL && traverse->right == NULL))
		{
				if (i >= limit - 1)
				{
						limit += 10;
						*codes = realloc (*codes, limit);
				}
				if (traverse -> left == NULL)
				{
						(*codes)[i] = '0';
						traverse = traverse -> right;
				}

				else
				{
						(*codes)[i] = '1';
						traverse = traverse -> left;
				}

				i++;
		}
		
		if (i >= limit - 1)
		{
				limit += 1;
				*codes = realloc (*codes, limit);
		}

		(*codes)[i] = '\0';
		return;
}

void strrev (char *str)
{
		/* Declarations */
		char temp;
		int i = 0, j = strlen(str) - 1;

		if ((strlen (str) == 1) || (strlen (str) == 0))
		{
				return;
		}

		for (; i < j; i++, j--)
		{
				temp = str[i];
				str[i] = str[j];
				str[j] = temp;
		}
}

int my_cmp2 (const void *data1, const void *data2)
{
		trees *data11 = (trees *)data1;
		trees *data22 = (trees *)data2;

		if (((trees **)data1)[0]->freq > ((trees **)data2)[0]->freq)
		{
				return 1;
		}

		else
		{
				return -1;
		}
}

int code_cmp(char *codes, trees **root, freq_table *code_table)
{
		int i, len = strlen (codes);

		for (i = 0; i < 256; i++)
		{
				if ((root[i] -> freq != 0) && len == strlen (code_table[i].codes))
				{
						if (strcmp(code_table[i].codes, codes) == 0)
						{
								return (i + 1);
						}
				}
		}

		return 0;
}

void printbits (char temp)
{
		int i;

		for (i = 0; i < 8; i++)
		{
				if ((temp & ((1 << 7) >> i)) == 0)
				{
						printf ("0");
				}
				else 
				{
						printf ("1");
				}
		}
		printf ("\n");
}
