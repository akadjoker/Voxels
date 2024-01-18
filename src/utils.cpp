/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: djoker <djoker@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/17 13:56:07 by lrosa-do          #+#    #+#             */
/*   Updated: 2024/01/17 17:44:49 by djoker           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "utils.hpp"

void Log(int severity, const char* fmt, ...)
{

		const char* type;
		const char* color;
		switch (severity) 
        {
			case  0:
				type = "info";
				color = CONSOLE_COLOR_GREEN;
				break;
			case 1:
				type = "warning";
				color = CONSOLE_COLOR_PURPLE;
				break;
            case 2:
				type = "error";
				color = CONSOLE_COLOR_RED;
				break;
			default:
				break;
		}


		time_t rawTime;
		struct tm* timeInfo;
		char timeBuffer[80];

		time(&rawTime);
		timeInfo = localtime(&rawTime);

		strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S]", timeInfo);

		char consoleFormat[1024];
		snprintf(consoleFormat, 1024, "%s%s %s%s%s: %s\n", CONSOLE_COLOR_CYAN,
				timeBuffer, color, type, CONSOLE_COLOR_RESET, fmt);


		char fileFormat[1024];
		snprintf(fileFormat, 1024, "%s %s: %s\n", timeBuffer, type, fmt);

		va_list argptr;

		va_start(argptr, fmt);
		vprintf(consoleFormat, argptr);
		va_end(argptr);

		char buf[4096];

		va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
		va_end(argptr);
}

 std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
{
		size_t start = 0;
		size_t end = string.find_first_of(delimiters);

		std::vector<std::string> result;

		while (end <= std::string::npos)
            {
			std::string token = string.substr(start, end - start);
			if (!token.empty())
			{
				result.push_back(token);
			}

			if (end == std::string::npos)
                {
				break;
			}

			start = end + 1;
			end = string.find_first_of(delimiters, start);
		}

		return result;

}
 bool find_word(const std::string& haystack,const std::string& needle)
{
    int index = (int)haystack.find(needle);
    if (index == (int)std::string::npos) return false;

    auto not_part_of_word = [&](int index){
        if (index < 0 || index >= (int)haystack.size()) return true;
        if (std::isspace(haystack[index]) || std::ispunct(haystack[index])) return true;
        return false;
    };
    return not_part_of_word(index-1) && not_part_of_word(index+needle.size());
}



 unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead)
{
   unsigned char *data = NULL;
    *bytesRead = 0;

    if (fileName != NULL)
    {
        SDL_RWops* file= SDL_RWFromFile(fileName, "rb");


        if (file != NULL)
        {
           unsigned int size =(int) SDL_RWsize(file);

            if (size > 0)
            {
                data = (unsigned char *)malloc(size*sizeof(unsigned char));

                unsigned int count = (unsigned int) SDL_RWread(file, data, sizeof(unsigned char), size);
                *bytesRead = count;

                if (count != size) Log(1, "FILEIO: [%s] File partially loaded", fileName);
                else  Log(0, "FILEIO: [%s] File loaded successfully", fileName);
            }
            else Log(1, "FILEIO: [%s] Failed to read file", fileName);
            SDL_RWclose(file);
        }
        else Log(1, "FILEIO: [%s] Failed to open file", fileName);
    }
    else Log(1, "FILEIO: File name provided is not valid");
    return data;
}

 bool SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite)
{
    bool success = false;

    if (fileName != NULL)
    {
        SDL_RWops* file= SDL_RWFromFile(fileName, "wb");
        if (file != NULL)
        {
             unsigned int count = (unsigned int) SDL_RWwrite(file, data, sizeof(unsigned char), bytesToWrite);
             if (count == 0) Log(2, "FILEIO: [%s] Failed to write file", fileName);
            else if (count != bytesToWrite) Log(1, "FILEIO: [%s] File partially written", fileName);
            else  Log(0, "FILEIO: [%s] File saved successfully", fileName);

            int result = SDL_RWclose(file);
            if (result == 0) success = true;
        }
        else Log(2, "FILEIO: [%s] Failed to open file", fileName);
    }
    else Log(2, "FILEIO: File name provided is not valid");

    return success;
}

 char *LoadFileText(const char *fileName)
{
    char *text = NULL;

    if (fileName != NULL)
    {
        SDL_RWops* textFile= SDL_RWFromFile(fileName, "rt");
        if (textFile != NULL)
        {
          unsigned  int size =(int) SDL_RWsize(textFile);
            if (size > 0)
            {
                text = (char *)malloc((size + 1)*sizeof(char));
                unsigned int count = (unsigned int) SDL_RWread(textFile, text, sizeof( char), size);
                if (count < size) text = (char*)realloc(text, count + 1);
                text[count] = '\0';

                 Log(0, "FILEIO: [%s] Text file loaded successfully", fileName);
            }
            else Log(2, "FILEIO: [%s] Failed to read text file", fileName);

            SDL_RWclose(textFile);
        }
        else Log(2, "FILEIO: [%s] Failed to open text file", fileName);
    }
    else Log(2, "FILEIO: File name provided is not valid");

    return text;
}

 bool SaveFileText(const char *fileName, char *text)
{
    bool success = false;

    if (fileName != NULL)
    {
         SDL_RWops* file= SDL_RWFromFile(fileName, "wt");
        if (file != NULL)
        {
             size_t strLen = SDL_strlen( text );
	         int count = SDL_RWwrite( file, text, 1, strLen );
            if (count < 0) Log(3, "FILEIO: [%s] Failed to write text file", fileName);
            else Log(0, "FILEIO: [%s] Text file saved successfully", fileName);

            int result = SDL_RWclose(file);
            if (result == 0) success = true;
        }
        else Log(2, "FILEIO: [%s] Failed to open text file", fileName);
    }
    else Log(2, "FILEIO: File name provided is not valid");
    return success;
}




//----------------------------------------------------------------------------------
// Text strings management functions
//----------------------------------------------------------------------------------
 const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

 unsigned int TextLength(const char *text)
{
    unsigned int length = 0; //strlen(text)
    if (text != NULL)
    {
        while (*text++) length++;
    }

    return length;
}

 const char *TextFormat(const char *text, ...)
{

    #define MAX_TEXTFORMAT_BUFFERS 4        // Maximum number of static buffers for text formatting
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int  index = 0;
    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using
    va_list args;
    va_start(args, text);
    vsprintf(currentBuffer, text, args);
    va_end(args);
    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

 int TextToInteger(const char *text)
{
    int value = 0;
    int sign = 1;

    if ((text[0] == '+') || (text[0] == '-'))
    {
        if (text[0] == '-') sign = -1;
        text++;
    }
    for (int i = 0; ((text[i] >= '0') && (text[i] <= '9')); ++i) value = value*10 + (int)(text[i] - '0');
    return value*sign;
}



 int TextCopy(char *dst, const char *src)
{
    int bytes = 0;

    if (dst != NULL)
    {
        while (*src != '\0')
        {
            *dst = *src;
            dst++;
            src++;

            bytes++;
        }

        *dst = '\0';
    }

    return bytes;
}

 bool TextIsEqual(const char *text1, const char *text2)
{
    bool result = false;

    if (strcmp(text1, text2) == 0) result = true;

    return result;
}


 const char *TextSubtext(const char *text, int position, int length)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    int textLength = TextLength(text);

    if (position >= textLength)
    {
        position = textLength - 1;
        length = 0;
    }

    if (length >= textLength) length = textLength;

    for (int c = 0 ; c < length ; c++)
    {
        *(buffer + c) = *(text + position);
        text++;
    }

    *(buffer + length) = '\0';

    return buffer;
}

 char *TextReplace(char *text, const char *replace, const char *by)
{
    // Sanity checks and initialization
    if (!text || !replace || !by) return NULL;

    char *result;

    char *insertPoint;      // Next insert point
    char *temp;             // Temp pointer
    int replaceLen;         // Replace string length of (the string to remove)
    int byLen;              // Replacement length (the string to replace replace by)
    int lastReplacePos;     // Distance between replace and end of last replace
    int count;              // Number of replacements

    replaceLen = TextLength(replace);
    if (replaceLen == 0) return NULL;  // Empty replace causes infinite loop during count

    byLen = TextLength(by);

    // Count the number of replacements needed
    insertPoint = text;
    for (count = 0; (temp = strstr(insertPoint, replace)); count++) insertPoint = temp + replaceLen;

    // Allocate returning string and point temp to it
    temp = result =(char*) malloc(TextLength(text) + (byLen - replaceLen)*count + 1);

    if (!result) return NULL;   // Memory could not be allocated

    // First time through the loop, all the variable are set correctly from here on,
    //    temp points to the end of the result string
    //    insertPoint points to the next occurrence of replace in text
    //    text points to the remainder of text after "end of replace"
    while (count--)
    {
        insertPoint = strstr(text, replace);
        lastReplacePos = (int)(insertPoint - text);
        temp = strncpy(temp, text, lastReplacePos) + lastReplacePos;
        temp = strcpy(temp, by) + byLen;
        text += lastReplacePos + replaceLen; // Move to next "end of replace"
    }

    // Copy remaind text part after replacement to result (pointed by moving temp)
    strcpy(temp, text);

    return result;
}


 char *TextInsert(const char *text, const char *insert, int position)
{
    int textLen = TextLength(text);
    int insertLen =  TextLength(insert);

    char *result = (char *)malloc(textLen + insertLen + 1);

    for (int i = 0; i < position; i++) result[i] = text[i];
    for (int i = position; i < insertLen + position; i++) result[i] = insert[i];
    for (int i = (insertLen + position); i < (textLen + insertLen); i++) result[i] = text[i];

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
}


 const char *TextJoin(const char **textList, int count, const char *delimiter)
{
    static char text[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(text, 0, MAX_TEXT_BUFFER_LENGTH);
    char *textPtr = text;

    int totalLength = 0;
    int delimiterLen = TextLength(delimiter);

    for (int i = 0; i < count; i++)
    {
        int textLength = TextLength(textList[i]);

        // Make sure joined text could fit inside MAX_TEXT_BUFFER_LENGTH
        if ((totalLength + textLength) < MAX_TEXT_BUFFER_LENGTH)
        {
            memcpy(textPtr, textList[i], textLength);
            totalLength += textLength;
            textPtr += textLength;

            if ((delimiterLen > 0) && (i < (count - 1)))
            {
                memcpy(textPtr, delimiter, delimiterLen);
                totalLength += delimiterLen;
                textPtr += delimiterLen;
            }
        }
    }

    return text;
}



 const char **TextSplit(const char *text, char delimiter, int *count)
{
    static const char *result[MAX_TEXTSPLIT_COUNT] = { NULL };
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };
    memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH);

    result[0] = buffer;
    int counter = 0;

    if (text != NULL)
    {
        counter = 1;

        // Count how many substrings we have on text and point to every one
        for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
        {
            buffer[i] = text[i];
            if (buffer[i] == '\0') break;
            else if (buffer[i] == delimiter)
            {
                buffer[i] = '\0';   // Set an end of string at this point
                result[counter] = buffer + i + 1;
                counter++;

                if (counter == MAX_TEXTSPLIT_COUNT) break;
            }
        }
    }

    *count = counter;
    return result;
}


 void TextAppend(char *text, const char *append, int *position)
{
    strcpy(text + *position, append);
    *position += TextLength(append);
}

 int TextFindIndex(const char *text, const char *find)
{
    int position = -1;

    char *ptr = (char*)strstr(text, find);

    if (ptr != NULL) position = (int)(ptr - text);

    return position;
}


 const char *TextToUpper(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)toupper(text[i]);
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}


 const char *TextToLower(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    for (int i = 0; i < MAX_TEXT_BUFFER_LENGTH; i++)
    {
        if (text[i] != '\0')
        {
            buffer[i] = (char)tolower(text[i]);
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}


 const char *TextToPascal(const char *text)
{
    static char buffer[MAX_TEXT_BUFFER_LENGTH] = { 0 };

    buffer[0] = (char)toupper(text[0]);

    for (int i = 1, j = 1; i < MAX_TEXT_BUFFER_LENGTH; i++, j++)
    {
        if (text[j] != '\0')
        {
            if (text[j] != '_') buffer[i] = text[j];
            else
            {
                j++;
                buffer[i] = (char)toupper(text[j]);
            }
        }
        else { buffer[i] = '\0'; break; }
    }

    return buffer;
}

//----------------------------------------------------------------------------------
// Files management functions
//----------------------------------------------------------------------------------

 bool FileExists(const char *fileName)
{
    bool result = false;
   if (access(fileName, F_OK) != -1) result = true;
    return result;
}

 const char *GetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return (dot + 1);
}


 const char *GetFileName(const char *filePath)
{
    const char *fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (!fileName) return filePath;

    return fileName + 1;
}



 const char *GetFileNameWithoutExt(const char *filePath)
{
    #define MAX_FILENAMEWITHOUTEXT_LENGTH   128

    static char fileName[MAX_FILENAMEWITHOUTEXT_LENGTH];
    memset(fileName, 0, MAX_FILENAMEWITHOUTEXT_LENGTH);

    if (filePath != NULL) strcpy(fileName, GetFileName(filePath));   // Get filename with extension

    int len = (int)strlen(fileName);

    for (int i = 0; (i < len) && (i < MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }

    return fileName;
}







 bool IsFileExtension(const char *fileName, const char *ext)
{
    bool result = false;
    const char *fileExt = GetFileExtension(fileName);

    if (fileExt != NULL)
    {

        int extCount = 0;
        const char **checkExts = TextSplit(ext, ';', &extCount);

        char fileExtLower[16] = { 0 };
        strcpy(fileExtLower, TextToLower(fileExt));

        for (int i = 0; i < extCount; i++)
        {
            if (TextIsEqual(fileExtLower, TextToLower(checkExts[i] + 1)))
            {
                result = true;
                break;
            }
        }
    }

    return result;
}


unsigned int g_seed = 0;

void Random_Seed(const int seed)
{
    if (!seed) {
    g_seed = SDL_GetTicks();
    } else {
    g_seed = seed;
    }
}

int Random_Int(const int min, const int max)
    {
    g_seed = 214013 * g_seed + 2531011;
    return min + (g_seed ^ g_seed >> 15) % (max - min + 1);
}

float Random_Float(const float min, const float max) 
{
    g_seed = 214013 * g_seed + 2531011;
    return min + (g_seed >> 16) * (1.0f / 65535.0f) * (max - min);
}



// Encode codepoint into utf8 text (char array length returned as parameter)
// NOTE: It uses a static array to store UTF-8 bytes
const char *CodepointToUTF8(int codepoint, int *utf8Size)
{
    static char utf8[6] = { 0 };
    int size = 0;   // Byte size of codepoint

    if (codepoint <= 0x7f)
    {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff)
    {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff)
    {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff)
    {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >>  6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *utf8Size = size;

    return utf8;
}


// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When an invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: The standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
int GetCodepoint(const char *text, int *codepointSize)
{
/*
    UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int codepoint = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *codepointSize = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        codepoint = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            codepoint = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *codepointSize = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *codepointSize = 2; return codepoint; }

        if ((octet >= 0xe0) && (octet <= 0xef))
        {
            codepoint = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *codepointSize = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return codepoint;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *codepointSize = 4; return codepoint; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            codepoint = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *codepointSize = 4;
        }
    }

    if (codepoint > 0x10ffff) codepoint = 0x3f;     // Codepoints after U+10ffff are invalid

    return codepoint;
}

// Get next codepoint in a byte sequence and bytes processed
int GetCodepointNext(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0]))
    {
        // 4 byte UTF-8 codepoint
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } //10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0]))
    {
        // 3 byte UTF-8 codepoint */
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } //10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0]))
    {
        // 2 byte UTF-8 codepoint
        if((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } //10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0]))
    {
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

// Get previous codepoint in a byte sequence and bytes processed
int GetCodepointPrevious(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    int cpSize = 0;
    *codepointSize = 0;

    // Move to previous codepoint
    do ptr--;
    while (((0x80 & ptr[0]) != 0) && ((0xc0 & ptr[0]) ==  0x80));

    codepoint = GetCodepointNext(ptr, &cpSize);

    if (codepoint != 0) *codepointSize = cpSize;

    return codepoint;
}

int GetCodepointCount(const char *text)
{
    unsigned int length = 0;
    char *ptr = (char *)&text[0];

    while (*ptr != '\0')
    {
        int next = 0;
        GetCodepointNext(ptr, &next);

        ptr += next;

        length++;
    }

    return length;
}


//*******************************************************************************************


enum FrustumSide
{
	RIGHT	= 0,		
	LEFT	= 1,		
	BOTTOM	= 2,		
	TOP		= 3,		
	BACK	= 4,		
	FRONT	= 5			
}; 


enum PlaneData
{
	A = 0,				
	B = 1,				
	C = 2,				
	D = 3				
};

void Frustum::NormalizePlane(int side)
{
	float magnitude = (float)sqrt( m_Frustum[side][A] * m_Frustum[side][A] + 
								   m_Frustum[side][B] * m_Frustum[side][B] + 
								   m_Frustum[side][C] * m_Frustum[side][C] );

	m_Frustum[side][A] /= magnitude;
	m_Frustum[side][B] /= magnitude;
	m_Frustum[side][C] /= magnitude;
	m_Frustum[side][D] /= magnitude; 
}

void Frustum::CalculateFrustum(const float *view, const float *proj)
{

	float   clip[16];	
    
	clip[ 0] = view[ 0] * proj[ 0] + view[ 1] * proj[ 4] + view[ 2] * proj[ 8] + view[ 3] * proj[12];
	clip[ 1] = view[ 0] * proj[ 1] + view[ 1] * proj[ 5] + view[ 2] * proj[ 9] + view[ 3] * proj[13];
	clip[ 2] = view[ 0] * proj[ 2] + view[ 1] * proj[ 6] + view[ 2] * proj[10] + view[ 3] * proj[14];
	clip[ 3] = view[ 0] * proj[ 3] + view[ 1] * proj[ 7] + view[ 2] * proj[11] + view[ 3] * proj[15];

	clip[ 4] = view[ 4] * proj[ 0] + view[ 5] * proj[ 4] + view[ 6] * proj[ 8] + view[ 7] * proj[12];
	clip[ 5] = view[ 4] * proj[ 1] + view[ 5] * proj[ 5] + view[ 6] * proj[ 9] + view[ 7] * proj[13];
	clip[ 6] = view[ 4] * proj[ 2] + view[ 5] * proj[ 6] + view[ 6] * proj[10] + view[ 7] * proj[14];
	clip[ 7] = view[ 4] * proj[ 3] + view[ 5] * proj[ 7] + view[ 6] * proj[11] + view[ 7] * proj[15];

	clip[ 8] = view[ 8] * proj[ 0] + view[ 9] * proj[ 4] + view[10] * proj[ 8] + view[11] * proj[12];
	clip[ 9] = view[ 8] * proj[ 1] + view[ 9] * proj[ 5] + view[10] * proj[ 9] + view[11] * proj[13];
	clip[10] = view[ 8] * proj[ 2] + view[ 9] * proj[ 6] + view[10] * proj[10] + view[11] * proj[14];
	clip[11] = view[ 8] * proj[ 3] + view[ 9] * proj[ 7] + view[10] * proj[11] + view[11] * proj[15];

	clip[12] = view[12] * proj[ 0] + view[13] * proj[ 4] + view[14] * proj[ 8] + view[15] * proj[12];
	clip[13] = view[12] * proj[ 1] + view[13] * proj[ 5] + view[14] * proj[ 9] + view[15] * proj[13];
	clip[14] = view[12] * proj[ 2] + view[13] * proj[ 6] + view[14] * proj[10] + view[15] * proj[14];
	clip[15] = view[12] * proj[ 3] + view[13] * proj[ 7] + view[14] * proj[11] + view[15] * proj[15];
	
	
	
	m_Frustum[RIGHT][A] = clip[ 3] - clip[ 0];
	m_Frustum[RIGHT][B] = clip[ 7] - clip[ 4];
	m_Frustum[RIGHT][C] = clip[11] - clip[ 8];
	m_Frustum[RIGHT][D] = clip[15] - clip[12];

	NormalizePlane( RIGHT);
    
    m_Frustum[LEFT][A] = clip[ 3] + clip[ 0];
	m_Frustum[LEFT][B] = clip[ 7] + clip[ 4];
	m_Frustum[LEFT][C] = clip[11] + clip[ 8];
	m_Frustum[LEFT][D] = clip[15] + clip[12];

	// Normalize the LEFT side
	NormalizePlane( LEFT);

	// This will extract the BOTTOM side of the frustum
	m_Frustum[BOTTOM][A] = clip[ 3] + clip[ 1];
	m_Frustum[BOTTOM][B] = clip[ 7] + clip[ 5];
	m_Frustum[BOTTOM][C] = clip[11] + clip[ 9];
	m_Frustum[BOTTOM][D] = clip[15] + clip[13];

	// Normalize the BOTTOM side
	NormalizePlane( BOTTOM);

	// This will extract the TOP side of the frustum
	m_Frustum[TOP][A] = clip[ 3] - clip[ 1];
	m_Frustum[TOP][B] = clip[ 7] - clip[ 5];
	m_Frustum[TOP][C] = clip[11] - clip[ 9];
	m_Frustum[TOP][D] = clip[15] - clip[13];

	// Normalize the TOP side
	NormalizePlane( TOP);

	// This will extract the BACK side of the frustum
	m_Frustum[BACK][A] = clip[ 3] - clip[ 2];
	m_Frustum[BACK][B] = clip[ 7] - clip[ 6];
	m_Frustum[BACK][C] = clip[11] - clip[10];
	m_Frustum[BACK][D] = clip[15] - clip[14];

	// Normalize the BACK side
	NormalizePlane( BACK);

	// This will extract the FRONT side of the frustum
	m_Frustum[FRONT][A] = clip[ 3] + clip[ 2];
	m_Frustum[FRONT][B] = clip[ 7] + clip[ 6];
	m_Frustum[FRONT][C] = clip[11] + clip[10];
	m_Frustum[FRONT][D] = clip[15] + clip[14];

	// Normalize the FRONT side
	NormalizePlane( FRONT);

}


bool Frustum::PointInView(float x, float y, float z)
{
    for(int i = 0; i < 6; i++ )
	{
		if(m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= 0)
		{
			return false;
		}
	}
   return true;
}

bool Frustum::SphereInView(float x, float y, float z, float radius)
{
    for(int i = 0; i < 6; i++ )
    {
        if(m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= -radius)
        {
            return false;
        }
    }

   return true;
}

bool Frustum::BoundingBoxInView(const BoundingBox &box)
{
    for(int i = 0; i < 6; i++ )
    {
        if(m_Frustum[i][A] * box.min.x + m_Frustum[i][B] * box.min.y + m_Frustum[i][C] * box.min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.max.x + m_Frustum[i][B] * box.min.y + m_Frustum[i][C] * box.min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.min.x + m_Frustum[i][B] * box.max.y + m_Frustum[i][C] * box.min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.max.x + m_Frustum[i][B] * box.max.y + m_Frustum[i][C] * box.min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.min.x + m_Frustum[i][B] * box.min.y + m_Frustum[i][C] * box.max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.max.x + m_Frustum[i][B] * box.min.y + m_Frustum[i][C] * box.max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.min.x + m_Frustum[i][B] * box.max.y + m_Frustum[i][C] * box.max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * box.max.x + m_Frustum[i][B] * box.max.y + m_Frustum[i][C] * box.max.z + m_Frustum[i][D] > 0)
            continue;
        return false;
    }

    return true;


   
}


bool Frustum::CubeInView( float x, float y, float z, float size )
{
	for(int i = 0; i < 6; i++ )
	{
		if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
		   continue;
		if(m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
		   continue;
		return false;
	}

	return true;
}

bool Frustum::CubeInView( const Vector3 &b_min, const Vector3 &b_max  )
{
    for(int i = 0; i < 6; i++ )
    {
        if(m_Frustum[i][A] * b_min.x + m_Frustum[i][B] * b_min.y + m_Frustum[i][C] * b_min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_max.x + m_Frustum[i][B] * b_min.y + m_Frustum[i][C] * b_min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_min.x + m_Frustum[i][B] * b_max.y + m_Frustum[i][C] * b_min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_max.x + m_Frustum[i][B] * b_max.y + m_Frustum[i][C] * b_min.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_min.x + m_Frustum[i][B] * b_min.y + m_Frustum[i][C] * b_max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_max.x + m_Frustum[i][B] * b_min.y + m_Frustum[i][C] * b_max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_min.x + m_Frustum[i][B] * b_max.y + m_Frustum[i][C] * b_max.z + m_Frustum[i][D] > 0)
            continue;
        if(m_Frustum[i][A] * b_max.x + m_Frustum[i][B] * b_max.y + m_Frustum[i][C] * b_max.z + m_Frustum[i][D] > 0)
            continue;
        return false;
    }

    return true;

}
