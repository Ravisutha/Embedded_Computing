/* 
Author 		: Ravisutha Sakrepatna Srinivasamurthy
Assignment	: Huffman Codec
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define ERR -1
#define SUCCESS 0
#define NUM_SYM 256

typedef struct 
{
		int freq;
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
int huff_compress (FILE *fp_read, trees **root, char *filename);
void get_codes (char **codes, unsigned char ch, trees **root);
void strrev (char *str);
void printbits (char temp);
long int size_of_file (FILE *fp);
void write_table (FILE *fp, trees **root);

int main (int argc, char *argv[])
{
		/* Declarations */
		FILE 			*fp;																	/* File pointer */
		unsigned int 	*freq = (unsigned int *)calloc (NUM_SYM, sizeof (unsigned int)); 		/* Allocate 256 bytes of memory to store frequency of symbols*/
		unsigned char 	*data = (unsigned char *)calloc (NUM_SYM, 1);
		freq_table 		*table = (freq_table *)calloc (NUM_SYM, sizeof (freq_table));			/* Create table */
		char 			*filename;
		int 			i;
		trees 			**root = calloc (256, sizeof (trees *));
		long int		size;

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

		/* Check if memory is not allocated */
		if (freq == NULL || filename == NULL)
		{
				perror ("table");
				return ERR;
		}

		/* Open file */
		if ((fp = fopen (filename, "rb")) == NULL)
		{
				printf ("%s:\n", filename);
				perror ("File op:");
				return ERR;
		}

		/* Fill data */
		fill_data (table);

		/* Get frequencies of symbols*/
		get_frequencies (fp, table);
		printf ("Successful file read and frequency table construction...\n");
		sleep (1);

		/* Buid the tree using table */
		build_tree (root, table);
		printf ("Tree building complete...\n");
		sleep (1);


		/* Compress the given file */
		huff_compress (fp, root, "compressed");
		printf ("Compression successful...\n");
		sleep (1);

		fcloseall ();
		return SUCCESS;
}

void get_frequencies (FILE *fp, freq_table *table)
{
		/* Declarations */
		unsigned char data;

		/* Calculate the frequency of occurance */
		while (fread (&data, 1, 1, fp) > 0)
		{
				table[data].freq += 1;
		}

		return;
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

void print_frequency (trees **data)
{
		int i;

		for (i = 0; i < 256; i++)
		{
				if (data[i] -> freq != 0)
				{
						printf ("%c: %d\n", (char)i, data[i] -> freq);
				}
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

int build_tree (trees **root, freq_table *table)
{
		/* Declarations */
		int i;
		int (*func_ptr)(const void *, const void*) = &my_cmp2;
		trees **symbol = (trees **)calloc (256, sizeof(trees *));

		get_symbol (symbol, root, table);
		qsort (symbol, NUM_SYM, sizeof (trees *), func_ptr);

		for (i = 0; i < 255; i++)
		{
				symbol[i + 1] = connect_node (symbol[i], symbol[i + 1]);
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

int huff_compress (FILE *fp_read, trees **root, char *filename)
{
		/* Declarations */
		FILE *fp_write = fopen (filename, "wb");
		unsigned char ch;
		int num = (char)0;
		char *codes;
		int i = 8, j = 0, k;
		long int size;

		/* Open the compression file */
		if (fp_write == NULL)
		{
				printf ("Filename : %s\n", filename);
				perror ("Opening");
		}

		size = size_of_file (fp_read);
		fwrite (&size, 1, sizeof (long int), fp_write);

		fseek (fp_read, 0, SEEK_SET);
		write_table (fp_write, root);

		for (k = 0; k < 256; k++)
		{
				get_codes (&codes, (char)k, root);
				strrev (codes);
		}

		i = 8;
		while (fread (&ch, 1, 1, fp_read) > 0)
		{
				j = 0;
				get_codes (&codes, ch, root);

				strrev (codes);

				while (1)
				{
						if (codes[j] == '\0')
						{
								break;
						}

						if (i == 0)
						{
								fwrite ((char *)&num, 1, 1, fp_write);
								num = 0;
								i = 8;
						}

						if (codes[j] == '1')
						{
								num += (1 << (i - 1));
						}

						i--;
						j++;
				}

				free(codes);
		}

		if (i != 8)
		{
				fwrite (&num, 1, 1, fp_write);
		}


		return SUCCESS;
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

long int size_of_file (FILE *fp)
{
		fseek (fp, 0, SEEK_SET);

		fseek (fp, 0, SEEK_END);

		return (ftell (fp));
}

void write_table (FILE *fp, trees **root)
{
		int i, freq;

		for (i = 0; i < 256; i++)
		{
				freq = root[i] -> freq;

				fwrite (&freq, 1, sizeof (int), fp);
		}

		return;
}

void change_file_name (char *filename, char *ext)
{
		/* Declarations */
		char *temp = calloc (1, strlen (filename) + 1);
		int i = 0;

		while (filename[i] != '\0')
		{
				if (filename[i] == '.')
				{
						break;
				}
				
				i++;
		}
		
		if (filename[i] == '.')
		{
				i++;
				for (; i < strlen ())
		}

		else
		{
				filename = realloc (filename, (strlen (filename) + strlen (ext) + 1));
		}
}
