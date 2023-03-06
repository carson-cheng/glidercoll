#define READ_FILE_LINE_OK 0
#define READ_FILE_LINE_EOF 1
#define READ_FILE_LINE_OVERFLOW 2
#define READ_FILE_LINE_FAIL 3

static never_inline int read_file_line (FILE *f, char *buf, i32 buf_size, int *result, int *has_newline)
{
	if (result)
		*result = READ_FILE_LINE_OK;
	if (has_newline)
		*has_newline = FALSE;
	
	if (!f || !buf || buf_size < 2)
	{
		if (f)
			fclose (f);
		if (result)
			*result = READ_FILE_LINE_FAIL;
		
		return ffsc (__func__);
	}
	
	if (!fgets (buf, buf_size, f))
	{
		if (result)
		{
			if (feof (f))
				*result = READ_FILE_LINE_EOF;
			else
				*result = READ_FILE_LINE_FAIL;
		}
		
		fclose (f);
		buf [0] = '\0';
		return FALSE;
	}
	
	i32 line_size = str_len (buf);
	
	if (line_size > 0 && buf [line_size - 1] != '\n')
	{
		if (fgetc (f) == EOF)
		{
			if (feof (f))
				return TRUE;
			
			if (result)
				*result = READ_FILE_LINE_FAIL;
		}
		else
			if (result)
				*result = READ_FILE_LINE_OVERFLOW;
		
		fclose (f);
		buf [0] = '\0';
		return FALSE;
	}
	
	if (line_size > 0 && buf [line_size - 1] == '\n')
	{
		line_size--;
		if (has_newline)
			*has_newline = TRUE;
	}
	
	if (line_size > 0 && buf [line_size - 1] == '\r')
		line_size--;
	
	buf [line_size] = '\0';
	return TRUE;
}
