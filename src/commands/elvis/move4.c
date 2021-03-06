/* move4.c */

/* Author:
 *	Steve Kirkendall
 *	16820 SW Tallac Way
 *	Beaverton, OR 97006
 *	kirkenda@jove.cs.pdx.edu, or ...uunet!tektronix!psueea!jove!kirkenda
 */


/* This file contains movement functions which are screen-relative */

#include "vi.h"

/* This moves the cursor to a particular row on the screen */
MARK moverow(m, cnt, key)
	MARK	m;	/* the cursor position */
	long	cnt;	/* the row we'll move to */
	int	key;	/* the keystroke of this move - H/L/M */
{
	DEFAULT(1);

	/* ignore off-screen counts */
	if (cnt > LINES - 1)
	{
		return MARK_UNSET;
	}

	/* calculate destination line based on key */
	cnt--;
	switch (key)
	{
	  case 'H':
		cnt = topline + cnt;
		break;

	  case 'M':
		cnt = topline + (LINES - 1) / 2;
		break;

	  case 'L':
		cnt = botline - cnt;
		break;
	}

	/* return the mark of the destination line */
	return MARK_AT_LINE(cnt);
}


/* This function repositions the current line to show on a given row */
MARK movez(m, cnt, key)
	MARK	m;	/* the cursor */
	long	cnt;	/* the line number we're repositioning */
	int	key;	/* key struck after the z */
{
	long	newtop;

	/* Which line are we talking about? */
	if (cnt < 0 || cnt > nlines)
	{
		return MARK_UNSET;
	}
	if (cnt)
	{
		m = MARK_AT_LINE(cnt);
		newtop = cnt;
	}
	else
	{
		newtop = markline(m);
	}

	/* figure out which line will have to be at the top of the screen */
	switch (key)
	{
	  case '\n':
	  case '\r':
	  case '+':
		break;

	  case '.':
	  case 'z':
		newtop -= LINES / 2;
		break;

	  case '-':
		newtop -= LINES - 1;
		break;

	  default:
		return MARK_UNSET;
	}

	/* make the new topline take effect */
	if (newtop >= 1)
	{
		topline = newtop;
	}
	else
	{
		topline = 1L;
	}
	mustredraw = TRUE;

	/* The cursor doesn't move */
	return m;
}


/* This function scrolls the screen.  It does this by calling redraw() with
 * an off-screen line as the argument.  It will move the cursor if necessary
 * so that the cursor is on the new screen.
 */
MARK movescroll(m, cnt, key)
	MARK	m;	/* the cursor position */
	long	cnt;	/* for some keys: the number of lines to scroll */
	int	key;	/* keystroke that causes this movement */
{
	MARK	tmp;	/* a temporary mark, used as arg to redraw() */

	/* adjust cnt, and maybe *o_scroll, depending of key */
	switch (key)
	{
	  case ctrl('F'):
	  case ctrl('B'):
		mustredraw = TRUE;
		cnt = LINES - 2; /* keeps one old line on screen */
		break;

	  case ctrl('E'):
	  case ctrl('Y'):
		DEFAULT(1);
		if (cnt > LINES - 1)
		{
			cnt = LINES - 1;
		}
		break;

	  case ctrl('U'):
	  case ctrl('D'):
		if (cnt == 0) /* default */
		{
			cnt = *o_scroll;
		}
		else
		{
			if (cnt > LINES - 1)
			{
				cnt = LINES - 1;
			}
			*o_scroll = cnt;
		}
		break;
	}

	/* scroll up or down, depending on key */
	switch (key)
	{
	  case ctrl('B'):
	  case ctrl('Y'):
	  case ctrl('U'):
		cnt = topline - cnt;
		if (cnt < 1L)
		{
			cnt = 1L;
		}
		tmp = MARK_AT_LINE(cnt) + markidx(m);
		redraw(tmp, FALSE);
		if (markline(m) > botline)
		{
			m = MARK_AT_LINE(botline);
		}
		break;

	  case ctrl('F'):
	  case ctrl('E'):
	  case ctrl('D'):
		cnt = botline + cnt;
		if (cnt > nlines)
		{
			cnt = nlines;
		}
		tmp = MARK_AT_LINE(cnt) + markidx(m);
		redraw(tmp, FALSE);
		if (markline(m) < topline)
		{
			m = MARK_AT_LINE(topline);
		}
		break;
	}

	/* arrange for ctrl-B and ctrl-F to redraw the smart line */
	if (key == ctrl('B') || key == ctrl('F'))
	{
		changes++;
	}

	return m;
}
