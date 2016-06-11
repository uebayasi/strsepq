/*
 * Copyright (c) 2016 Masao Uebayashi <uebayasi@tombiinc.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stddef.h>

char *
strsepq(char **stringp, const char *delim, const char quote)
{
	char *s, *token = NULL;
	enum { OUT, INQUOTE, INTOKEN } state = OUT;

	if (*stringp == NULL)
		return NULL;
	for (s = *stringp; *s != '\0'; s++) {
		const int c = *s;
		const char *dp;

		for (dp = delim; *dp != '\0'; dp++)
			if (*dp == c)
				break;

		enum { DELIM, QUOTE, ANY } input =
		    (*dp != '\0') ? DELIM :
		    (c == quote) ? QUOTE : ANY;

		switch (state) {
		case OUT:
			switch (input) {
			case DELIM:
				break;
			case QUOTE:
				/*
                                 * Start of a quoted string; only start of
                                 * token can be start of a quoted string.
				 */
				state = INQUOTE;
				*s++ = '\0';
				token = s;
				break;
			case ANY:
				state = INTOKEN;
				token = s;
				break;
			}
			break;
		case INQUOTE:
			switch (input) {
			case DELIM:
				break;
			case QUOTE:
				state = OUT;
				*s++ = '\0';
				break;
			case ANY:
				break;
			}
		case INTOKEN:
			switch (input) {
			case DELIM:
				state = OUT;
				break;
			case QUOTE:
				break;
			case ANY:
				break;
			}
		}
		if (state == OUT && token != NULL)
			break;
	}
	if (*s == '\0')
		*stringp = NULL;
	else if (token != NULL) {
		/*
                 * End of a quoted token; assume next char is a delim.
		 */
		*s = '\0';
		*stringp = s + 1;
	}
	return token;
}

#ifdef TEST
#include <stdio.h>
#include <string.h>

#ifndef nitems
#define nitems(x) (sizeof(x)/sizeof(x[0]))
#endif

const char *tests[] = {
	"a",
	" ",
	"\"",
	"\"\"",
	"a\"",
	"a \"",
	"aabbcc",
	"aa bb cc",
	"\"aa bb cc\"",
	"aa \"bb\" cc",
        "aa \"bb\"cc",  /* XXX 'c' after '"' is ignored as a delim */
	"a a\"bb\"c c",
	"a a\"b b\"c c",
};

int
main(int ac, char *av[])
{
	char buf[16], *p, *s;
	int i;

	for (i = 0; i < nitems(tests); i++) {
		printf("INPUT: '%s'\n", tests[i]);
		strlcpy(buf, tests[i], sizeof(buf));
		p = buf;
		while ((s = strsepq(&p, " \t", '"')) != NULL)
			printf(" TOKEN: '%s'\n", s);
	}
	return 0;
}
#endif
