#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "input.h"

struct widget_input *input_alloc(struct screen *scr, INT32 x, INT32 y,
	INT32 width, INT32 attr, CONST CHAR16 *def_text)
{
	struct widget_input *in;
	UINTN text_length;

	ASSERT(scr != NULL);

	in = AllocateZeroPool(sizeof(struct widget_input));
	if(!in)
		return NULL;

	in->win = newwin(scr, width, 1, x, y);
	if(!in->win) {
		input_release(in);
		return NULL;
	}
	wattrset(in->win, attr);
	whline(in->win, 0, 0, 0, attr, 0);
	mvwprintf(in->win, 0, 0, def_text);

	text_length = StrLen(def_text);
	in->point = text_length > width ? width : text_length;
	in->max_size = width;
	in->buf_len = in->point;
	in->buffer = AllocateCopyPool((in->max_size + 1) * sizeof(CHAR16), def_text);
	if(!in->buffer) {
		input_release(in);
		return NULL;
	}

	return in;
}

VOID input_release(struct widget_input *in)
{
	ASSERT(in != NULL);

	if(in->win)
		delwin(in->win);
	if(in->buffer)
		FreePool(in->buffer);

	FreePool(in);
}

VOID input_set_point(struct widget_input *in, INTN pos)
{
	in->point = pos;
	if(pos < 0)
		in->point = 0;
	else if(pos > in->buf_len)
		in->point = in->buf_len;

	wmove(in->win, in->point, 0);
	input_update(in);
}

VOID input_handle_char(struct widget_input *in, EFI_INPUT_KEY key)
{
	UINTN i;

	if(key.ScanCode == SCAN_LEFT)
		input_set_point(in, in->point - 1);
	else if(key.ScanCode == SCAN_RIGHT)
		input_set_point(in, in->point + 1);
	else {
		switch(key.UnicodeChar) {

		case 0x0:
			break;
		case CHAR_BACKSPACE: {
			if(in->buf_len == 0)
				break;

			for(i = in->point; i < in->buf_len; i++)
				in->buffer[i - 1] = in->buffer[i];
			in->buffer[i - 1] = CHAR_NULL;
			mvwaddch(in->win, i - 1, 0, L' ', -1);
			mvwprintf(in->win, in->point - 1, 0, in->buffer + in->point - 1);

			in->buf_len--;
			input_set_point(in, in->point - 1);
		}
		case CHAR_TAB: 
		case CHAR_LINEFEED:
		case CHAR_CARRIAGE_RETURN:
			break;
		default: {
			if(in->buf_len >= (in->max_size-1))
				break;
				
			for(i = in->max_size-1; i > in->point; i--)
				in->buffer[i] = in->buffer[i - 1];
			in->buffer[in->point] = key.UnicodeChar;
			wprintf(in->win, in->buffer + in->point);

			in->buf_len++;
			input_set_point(in, in->point + 1);
		}

		}
	}
}

VOID input_update(struct widget_input *in)
{
	wrefresh(in->win);
}

