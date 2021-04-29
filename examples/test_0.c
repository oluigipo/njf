#include <stdio.h>

#define NJF_IMPLEMENTATION
#include "njf.h"

int tab_count = 0;

void print_tabs(void)
{
	for (int i = 0; i < tab_count; ++i)
		printf("    ");
}

void print_body(NJF_Body* body)
{
	// Iterate through objects
	for (NJF_Object object = njf_first_object(body);
		 object.valid;
		 njf_next_object(&object))
	{
		print_tabs();
		printf("Object \"%.*s\" Flags: ", object.name.length, object.name.ptr);
		
		// Iterate through flags
		for (NJF_Flag flag = njf_first_flag(&object);
			 flag.kind != NJF_Kind_NotFound;
			 njf_next_flag(&flag))
		{
			switch (flag.kind)
			{
				case NJF_Kind_Ident: printf("%.*s", flag.as.ident.length, flag.as.ident.ptr); break;
				case NJF_Kind_Number: printf("%lli", flag.as.number); break;
			}
			
			putchar(' ');
		}
		
		putchar('\n');
		
		// Print body
		if (object.has_body)
		{
			tab_count++;
			
			NJF_Body nested_body = njf_object_body(&object);
			print_body(&nested_body);
			
			tab_count--;
		}
	}
}

int main(void)
{
	NJF_File file;
	if (!njf_load_file("test_0.njf", &file))
		return 1;
	
	print_body(&file.body);
	
	njf_free(&file);
	return 0;
}
