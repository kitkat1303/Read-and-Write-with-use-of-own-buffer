#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

using namespace std;

char decimal[100];

int recursive_itoa(int arg)
{
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if (div > 0)
	{
		index = recursive_itoa(div);
	}
	decimal[index] = mod + '0';
	return ++index;
}

char *itoa(const int arg)
{
	bzero(decimal, 100);
	int order = recursive_itoa(arg);
	char *new_decimal = new char[order + 1];
	bcopy(decimal, new_decimal, order + 1);
	return new_decimal;
}

int printf(const void *format, ...)
{
	va_list list;
	va_start(list, format);

	char *msg = (char *)format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0, k = 0;
	while (msg[i] != '\0')
	{
		if (msg[i] == '%' && msg[i + 1] == 'd')
		{
			buf[j] = '\0';
			nWritten += write(1, buf, j);
			j = 0;
			i += 2;

			int int_val = va_arg(list, int);
			char *dec = itoa(abs(int_val));
			if (int_val < 0)
			{
				nWritten += write(1, "-", 1);
			}
			nWritten += write(1, dec, strlen(dec));
			delete dec;
		}
		else
		{
			buf[j++] = msg[i++];
		}
	}
	if (j > 0)
	{
		nWritten += write(1, buf, j);
	}
	va_end(list);
	return nWritten;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
	{
		return -1;
	}
	stream->mode = mode;
	stream->pos = 0;
	if (stream->buffer != (char *)0 && stream->bufown == true)
	{
		delete stream->buffer;
	}

	switch (mode)
	{
	case _IONBF:
		stream->buffer = (char *)0;
		stream->size = 0;
		stream->bufown = false;
		break;
	case _IOLBF:
	case _IOFBF:
		if (buf != (char *)0)
		{
			stream->buffer = buf;
			stream->size = size;
			stream->bufown = false;
		}
		else
		{
			stream->buffer = new char[BUFSIZ];
			stream->size = BUFSIZ;
			stream->bufown = true;
		}
		break;
	}
	return 0;
}

void setbuf(FILE *stream, char *buf)
{
	setvbuf(stream, buf, (buf != (char *)0) ? _IOFBF : _IONBF, BUFSIZ);
}

FILE *fopen(const char *path, const char *mode)
{
	FILE *stream = new FILE();
	setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);

	// fopen( ) mode
	// r or rb = O_RDONLY
	// w or wb = O_WRONLY | O_CREAT | O_TRUNC
	// a or ab = O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b = O_RDWR
	// w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

	switch (mode[0])
	{
	case 'r':
		if (mode[1] == '\0') // r
		{
			stream->flag = O_RDONLY;
		}
		else if (mode[1] == 'b')
		{
			if (mode[2] == '\0') // rb
			{
				stream->flag = O_RDONLY;
			}
			else if (mode[2] == '+') // rb+
			{
				stream->flag = O_RDWR;
			}
		}
		else if (mode[1] == '+') // r+  r+b
		{
			stream->flag = O_RDWR;
		}
		break;
	case 'w':
		if (mode[1] == '\0') // w
		{
			stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		}
		else if (mode[1] == 'b')
		{
			if (mode[2] == '\0') // wb
			{
				stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
			}
			else if (mode[2] == '+') // wb+
			{
				stream->flag = O_RDWR | O_CREAT | O_TRUNC;
			}
		}
		else if (mode[1] == '+') // w+  w+b
		{
			stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		}
		break;
	case 'a':
		if (mode[1] == '\0') // a
		{
			stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		}
		else if (mode[1] == 'b')
		{
			if (mode[2] == '\0') // ab
			{
				stream->flag = O_WRONLY | O_CREAT | O_APPEND;
			}
			else if (mode[2] == '+') // ab+
			{
				stream->flag = O_RDWR | O_CREAT | O_APPEND;
			}
		}
		else if (mode[1] == '+') // a+  a+b
		{
			stream->flag = O_RDWR | O_CREAT | O_APPEND;
		}
		break;
	}

	mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

	if ((stream->fd = open(path, stream->flag, open_mode)) == -1)
	{
		delete stream;
		printf("fopen failed\n");
		stream = NULL;
	}

	return stream;
}

/**
 * @brief fpurge
 * this function takes in a FILE pointer and 
 * clear the input or output of the buffer 
 * 
 * @param stream : file object pointer
 * @return int : returns 0
 */
int fpurge(FILE *stream)
{
	// clears an input/output stream buffer
	// reset pos
	stream->pos = 0;
	bzero(stream->buffer, stream->size);
	return 0;
}

/**
 * @brief fflush 
 * this function takes in a FILE pointer and 
 * clear the input or output of the buffer after
 * writing what is in the buffer to the file
 * 
 * @param stream : file object pointer
 * @return int : return EOF if file is empty or 0 if succesful
 */
int fflush(FILE *stream)
{
	// synchronizes an output stream with the actual file
	if (stream->eof == true)
	{
		return (EOF);
	}
	write(stream->fd, stream->buffer, stream->size);
	// reset pos
	stream->pos = 0;
	bzero(stream->buffer, stream->size);
	stream->actual_size = 0;
	stream->eof = false;
	return 0;
}

/**
 * @brief fread
 * this function takes in a pointer, a size (number of bytes per element), 
 * nmemb (number of elements) and a file pointer
 * it then reads the number of bytes into the buffer
 * 
 * @param ptr : void pointer
 * @param size : number of bytes per element
 * @param nmemb : number of elements 
 * @param stream : file pointer
 * @return size_t : returns number of bytes read
 */
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	// number of bytes being read in from the file
	int numBytes = size * nmemb;
	int numRead = 1;
	int totalBytesRead = 0;
	int totalElementsRead = 0;
	// cast ptr
	char *buffer = (char *)ptr;

	// if last operation was w then move contents of buffer to file
	// pos should be 0
	if (stream->lastop == 'w')
	{
		fflush(stream);
	}

	// if number of bytes to be read is greater than buffer or
	// not using buffer
	if (numBytes >= stream->size || stream->mode != _IONBF)
	{
		stream->lastop == 'r';
		// read size for loop until number of elements
		totalBytesRead = read(stream->fd, buffer, numBytes);
		return totalBytesRead / size;
	}
	// if own buffer or buffer can fit number of bytes
	else
	{
		stream->lastop == 'r';
		totalBytesRead = read(stream->fd, stream->buffer, numBytes);
		// update position
		stream->pos += totalBytesRead;
		// update actual size of buffer used
		stream->actual_size = totalBytesRead;

		memcpy(buffer, stream->buffer, stream->actual_size);
		fflush(stream);

		// set if eof
		if (stream->pos >= stream->size)
		{
			stream->eof = true;
		}

		// return total number of elements
		return totalBytesRead / size;
	}
}

/**
 * @brief fwrite
 * this function takes in a pointer to what should be written to the file,
 * a size (number of bytes per element), 
 * nmemb (number of elements) and a file pointer
 * it then writes the number of bytes into the buffer
 * 
 * @param ptr : void pointer
 * @param size : number of bytes per element
 * @param nmemb : number of elements 
 * @param stream : file pointer
 * @return size_t : returns number of bytes read
 */
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	char *buffer = (char *)ptr;
	int numBytes = size * nmemb;
	int totalBytesWritten = 0;
	// int numFullBuffer = numBytes / stream->size;

	// reset pos to 0
	if (stream->lastop == 'r')
	{
		fflush(stream);
	}

	// if number of bytes to write is greater than buffer or
	// not using buffer
	if (numBytes >= stream->size || stream->mode != _IONBF)
	{
		stream->lastop = 'w';
		totalBytesWritten = write(stream->fd, buffer, numBytes);
		if(totalBytesWritten == 0){
			return EOF;
		}
		else{
			return totalBytesWritten / size;
		}
		
	}
	// if own buffer or buffer can hold size of write
	else
	{
		stream->lastop = 'w';
		// copy ptr to stream buffer
		memcpy(stream->buffer, buffer, numBytes);
		// ensure that when writing, use actual size
		totalBytesWritten = write(stream->fd, stream->buffer, numBytes);
		stream->pos += totalBytesWritten;

		// set eof if at eof
		if (stream->pos >= stream->size)
		{
			stream->eof = true;
		}
	if(totalBytesWritten == 0){
			return EOF;
		}
		else{
			return totalBytesWritten / size;
		}
	}
}

/**
 * @brief fgetc
 * this function takes in a file stream reads a single 
 * character into the buffer
 * 
 * @param stream : file pointer
 * @return int : returns EOF if no char to be read and the char as an int if 
 * it is succesful
 */
int fgetc(FILE *stream)
{
	// reads a character from a file stream
	if (stream->lastop == 'w')
	{
		fflush(stream);
	}
		char buffer;
		int numberRead;
		numberRead = read(stream->fd, &buffer, 1);
		if (numberRead == 0)
		{
			stream->eof = true;
			return EOF;
		}
		else
		{
			stream->lastop = 'r';
			return (int)buffer;
		}
	
}

/**
 * @brief fputc
 * this function takes in an int (char) and a file pointer and 
 * writes the character to the file
 * 
 * @param c : character to be written 
 * @param stream 
 * @return int : returns EOF if no char has been written or else 
 * returns char as int
 */
int fputc(int c, FILE *stream)
{
	// writes a character to a file stream
	if (stream->lastop == 'r')
	{
		fflush(stream);
	}

	char buffer = (char)c;
	int numberWrite;
	numberWrite = write(stream->fd, &buffer, 1);
	if (numberWrite == 0)
	{
		stream->eof = true;
		return EOF;
	}
	else
	{
		stream->lastop = 'w';
		return (int)buffer;
	}
}

/**
 * @brief fgets
 * this function takes in a char ptr, a size, and file pointer
 * and reads in a single line into the buffer
 * 
 * @param str : str to be filled
 * @param size : number of chars to be read
 * @param stream : file pointer
 * @return char* : returns NULL if no chars were read or else returns the 
 * string containing a line of chars up to n-1 or once the end line character 
 * is reached
 */
char *fgets(char *str, int size, FILE *stream)
{
	// reads a character string from a file stream
	// check if not at EOF or else return NULL
	if (stream->lastop == 'w')
	{
		fflush(stream);
	}

	int getChar;
	int numCharsRead = 0;

	// while characters are not \n or EOF add to str
	while (numCharsRead < size)
	{
		getChar = fgetc(stream);
		//if eof and read in char, return string 
		if (getChar == EOF)
		{
			if(numCharsRead == 0){
				return NULL;
			}
			else{
				//add NULL character and return string 
				stream->lastop = 'r';
				str[numCharsRead] = '\0';
				return str;
			}
		}
		//if end of line reached, return string 
		else if (getChar == '\n')
		{
			//reached end of line so add NULL character and return string
			stream->lastop = 'r';
			//add new line 
			str[numCharsRead] = getChar;
			str[numCharsRead+1] = '\0';
			return str;
		}
		//or else add character to str and increment numCharsRead
		else{
			str[numCharsRead] = getChar;
			numCharsRead++;
		}
		
	}

	stream->lastop = 'r';
	str[numCharsRead] = '\0';
	return str;
}

/**
 * @brief fputs
 * this function takes in a char ptr with data to be written, 
 * a size, and file pointer
 * and writes in a single line into the buffer
 * 
 * @param str : data to be written to file
 * @param stream : file pointer
 * @return int : returns NULL if no chars were written or else returns the 
 * the number or bytes written to the file
 */
int fputs(const char *str, FILE *stream)
{
	// writes a character string to a file stream
	if (stream->lastop == 'w')
	{
		fflush(stream);
	}

	int numWritten = 0;
	if (str == NULL)
	{
		return EOF;
	}

	while (*str != '\n')
	{

		if(*str == '\0'){
			return numWritten;
		}
		numWritten = fputc(*str, stream);
		str++;
	}
	stream->lastop = 'w';
	return numWritten;
}

/**
 * @brief feof
 * returns true if at end of buffer
 * 
 * @param stream : file pointer
 * @return int : true if at end of buffer
 */
int feof(FILE *stream)
{
	return stream->eof == true;
}

/**
 * @brief fseek
 * this function takes in a file poimter, an offset number, and a
 * whence that describes where the offset will be set from 
 * the function then sets the file pointer to that position and returns 
 * 0 if it is successful and -1 if it is not
 * 
 * @param stream : file object pointer
 * @param offset : number of bytes to offset by 
 * @param whence : what type of offset 
 * @return int : 0 if successful, -1 if not
 */
int fseek(FILE *stream, long offset, int whence)
{
	// moves the file position to a specific location in a file
	if (whence != SEEK_CUR && whence != SEEK_SET && whence != SEEK_END)
	{
		return -1;
	}
	fflush(stream);
	int location = lseek(stream->fd, offset, whence);
	stream->eof = false;
	printf("%d\n", location);

	if (location == -1)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief fclose
 * this function takes in a file pointer and closes the 
 * file 
 * 
 * @param stream : file object 
 * @return int : return EOF if no file associated or else return 
 * what the close system call returns
 */
int fclose(FILE *stream)
{
	// closes a file
	if (stream == NULL)
	{
		return (EOF);
	}

	fflush(stream);

	if (stream->bufown == true)
	{
		delete stream->buffer;
	}

	// return 0 is successful
	return close(stream->fd);
}
