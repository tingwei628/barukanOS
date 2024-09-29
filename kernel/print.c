#include "stdarg.h"
#include "print.h"
#include "memory.h"
#include "string.h"

static ScreenBuffer screen_buffer =
{
    .column = 0,
    .row = 0
};


static int32_t udecimal_to_string(uint8_t *buffer, uint32_t position, uint64_t digits)
{
    uint8_t digits_buffer[50];

    int32_t size = digits_to_string(digits_buffer, digits);

    for (int32_t i = size-1; i >= 0; i--)
    {
        buffer[position++] = digits_buffer[i];
    }

    return size;
}

static int32_t decimal_to_string(uint8_t *buffer, uint32_t position, int64_t digits)
{
    int32_t size = 0;

    if (digits < 0)
    {
        digits = -digits;
        buffer[position++] = '-';
        size = 1;
    }

    size += udecimal_to_string(buffer, position, (uint64_t)digits);
    return size;
}

static int32_t hex_to_string(uint8_t *buffer, uint32_t position, uint64_t digits)
{
    uint8_t digits_buffer[50];

    int32_t size  = digits_hex_to_string(digits_buffer, digits);

    for (int32_t i = size-1; i >= 0; i--)
    {
        buffer[position++] = digits_buffer[i];
    }

    buffer[position++] = 'H';

    return size + 1;
}

static int32_t read_string(uint8_t *buffer, uint32_t position, const uint8_t *src)
{
    int32_t index = 0;

    while (src[index] != '\0')
    {
        buffer[position++] = src[index];
        index ++;
    }

    return index;
}

static void write_screen(const uint8_t *buffer, uint32_t size, ScreenBuffer *strbuf, uint8_t color)
{
    int32_t column = strbuf->column;
    int32_t row = strbuf->row;
    uint8_t *buf = VGA_START;
    
    for (int32_t i = 0; i < size; i++)
    {
        //if scrolling screen
        if (row >= SCREEN_MAX_ROW)
        {
            kmemcpy(buf, buf + LINE_SIZE, LINE_SIZE * (SCREEN_MAX_ROW -1));
            kmemset(buf + LINE_SIZE * (SCREEN_MAX_ROW-1), 0, LINE_SIZE);
            row--;
        }
        
        // next line
        if (buffer[i] == '\n')
        {
            column = 0;
            row++;
        } 
        else
        {
            buf[column * 2 + row * LINE_SIZE] = buffer[i];
            buf[column * 2 + row * LINE_SIZE + 1] = color;


            column++;

            // next line
            if (column >= SCREEN_MAX_COL)
            {

                column=0;
                row++;
            }
        }
    }

    strbuf->column = column;
    strbuf->row = row;
}

uint32_t printk(const uint8_t *format, ...)
{
    uint8_t buffer[1024];
    uint32_t buffer_size = 0;
    int64_t integer = 0;
    uint8_t *string = 0;

    va_list args;

    va_start(args, format);

    for (int32_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%')
        {
            buffer[buffer_size++] = format[i];
        }
        else
        {
            switch (format[++i]) {
                case 'x':
                    integer = va_arg(args, int64_t);
                    buffer_size += hex_to_string(buffer, buffer_size, (uint64_t)integer);
                    break;

                case 'u':
                    integer = va_arg(args, int64_t);
                    buffer_size += udecimal_to_string(buffer, buffer_size, (uint64_t)integer);
                    break;

                case 'd':
                    integer = va_arg(args, int32_t);
                    buffer_size += decimal_to_string(buffer, buffer_size, integer);
                    break;

                case 's':
                    string = va_arg(args, uint8_t*);
                    buffer_size += read_string(buffer, buffer_size, string);
                    break;

                default:
                    buffer[buffer_size++] = '%';
                    i--;
            }
        }     
    }

    write_screen(buffer, buffer_size, &screen_buffer, COLOR_WHITE);
    va_end(args);

    return buffer_size;
}

