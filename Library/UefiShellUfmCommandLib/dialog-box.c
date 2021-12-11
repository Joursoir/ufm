#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "dialog-box.h"

#define COLOR_WHITE_CYAN (EFI_TEXT_ATTR(EFI_WHITE, EFI_CYAN))
#define COLOR_WHITE_LIGHTGRAY (EFI_TEXT_ATTR(EFI_WHITE, EFI_LIGHTGRAY))
#define HIGHLIGHT(w, active) \
	whline(w, 0, 0, 0, active ? COLOR_WHITE_CYAN : COLOR_WHITE_LIGHTGRAY, 0)

CONST CHAR16 ok_button_msg[] = L"[ OK ]";
CONST CHAR16 cl_button_msg[] = L"[ Cancel ]";

STATIC VOID highlight_widget(struct dbox_ctx *dbox, enum dbox_widgets wid, BOOLEAN active)
{
	if(wid == DBOX_OK_BUTTON) {
		HIGHLIGHT(dbox->wok, active);
		wrefresh(dbox->wok);
	}
	else if(wid == DBOX_CL_BUTTON) {
		HIGHLIGHT(dbox->wcl, active);
		wrefresh(dbox->wcl);
	}
	else if(wid == DBOX_INPUT_BOX)
		input_update(dbox->in);
}

STATIC VOID set_active_widget(struct dbox_ctx *dbox, enum dbox_widgets wid)
{
	enum dbox_widgets extreme_val = dbox->in ? DBOX_END_WID : DBOX_INPUT_BOX;

	if(wid <= DBOX_START_WID)
		wid = extreme_val - 1;
	else if(wid >= extreme_val)
		wid = DBOX_START_WID + 1;

	highlight_widget(dbox, dbox->active_widget, FALSE);
	dbox->active_widget = wid;
	highlight_widget(dbox, dbox->active_widget, TRUE);
}

struct dbox_ctx *dbox_alloc(struct screen *scr, CONST CHAR16 *title,
	CONST CHAR16 *label, BOOLEAN wid_input, CONST CHAR16 *input_text)
{
	struct dbox_ctx *dbox;
	UINTN y;
	CONST UINTN dbox_height = wid_input ? 6 : 5;
	CONST UINTN dbox_width = 52; // why 52?
	CONST UINTN len_ok = StrLen(ok_button_msg), len_cl = StrLen(cl_button_msg);
	UINTN start_x, start_y, start_ok;
	BOOLEAN res = FALSE;

	ASSERT(scr != NULL);

	dbox = AllocateZeroPool(sizeof(struct dbox_ctx));
	if(!dbox)
		return NULL;

	do { // START DO

	start_x = (scr->columns - dbox_width) / 2;
	start_y = (scr->lines - dbox_height) / 2;
	dbox->wbg = newwin(scr, dbox_width, dbox_height, start_x, start_y);
	if(!dbox->wbg)
		break;
	wborder(dbox->wbg,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL, BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_DOWN_RIGHT, BOXDRAW_DOWN_LEFT, BOXDRAW_UP_RIGHT, BOXDRAW_UP_LEFT
	);
	mvwprintf(dbox->wbg, 1 + ((dbox_width - StrLen(title)) / 2), 0, L"%s", title);
	mvwprintf(dbox->wbg, 2, 1, L"%s", label);
	whline(dbox->wbg, 1, dbox_height - 3, BOXDRAW_HORIZONTAL, -1, dbox_width - 2);

	if(wid_input) {
		dbox->in = input_alloc(scr, start_x + 2, start_y + 2,
			dbox_width - 4, COLOR_WHITE_CYAN, input_text);
		if(!dbox->in)
			break;
	}

	start_ok = start_x + 1 + ((dbox_width - len_ok - 1 - len_cl) / 2);
	dbox->wok = newwin(scr, len_ok, 1, start_ok, start_y + dbox_height - 2);
	if(!dbox->wok)
		break;
	mvwprintf(dbox->wok, 0, 0, ok_button_msg);
	HIGHLIGHT(dbox->wok, !wid_input);

	dbox->wcl = newwin(scr, len_cl, 1, start_ok + 1 + len_ok, start_y + dbox_height - 2);
	if(!dbox->wcl)
		break;
	mvwprintf(dbox->wcl, 0, 0, cl_button_msg);
	HIGHLIGHT(dbox->wcl, FALSE);

	for(y = 0; y < dbox_height; y++)
		whline(dbox->wbg, 0, y, 0, COLOR_WHITE_LIGHTGRAY, 0);

	res = TRUE;

	} while(0); // END DO

	if(!res) {
		dbox_release(dbox);
		return NULL;
	}

	return dbox;
}

VOID dbox_release(struct dbox_ctx *dbox)
{
	ASSERT(dbox != NULL);

	if(dbox->wbg)
		delwin(dbox->wbg);
	if(dbox->in)
		input_release(dbox->in);
	if(dbox->wok)
		delwin(dbox->wok);
	if(dbox->wcl)
		delwin(dbox->wcl);

	FreePool(dbox);
}

BOOLEAN dbox_handle(struct dbox_ctx *dbox)
{
	EFI_INPUT_KEY key;

	set_active_widget(dbox, dbox->in ? DBOX_INPUT_BOX : DBOX_OK_BUTTON);
	for(;;)
	{
		key = wgetch(dbox->wbg);

		if(key.ScanCode == SCAN_ESC)
			return FALSE;
		else if(key.ScanCode == SCAN_UP)
			set_active_widget(dbox, dbox->active_widget - 1);
		else if(key.ScanCode == SCAN_DOWN)
			set_active_widget(dbox, dbox->active_widget + 1);
		else if(key.UnicodeChar == CHAR_LINEFEED || key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
			if(dbox->active_widget == DBOX_CL_BUTTON)
				return FALSE;

			return TRUE;
		}
		else if(dbox->active_widget == DBOX_INPUT_BOX)
			input_handle_char(dbox->in, key);
	}
}

VOID dbox_refresh(struct dbox_ctx *dbox)
{
	wrefresh(dbox->wbg);
	if(dbox->in)
		input_update(dbox->in);
	wrefresh(dbox->wok);
	wrefresh(dbox->wcl);
}
