/* Flexible string comparison function with wildcard support - header
 * Version 1.23 (21/12/2007)
 * Copyright (C) 2007 Daniel Collins <solemnwarning@solemnwarning.net>
 *
 * Released to public domain, I take no responsibility for this code or it's
 * users, it may work, it may not, it may destroy the universe but it has no
 * warranty, so no sending lawyers my way if this code breaks somthing.
 *
 * If you find a bug please email me :)
*/

/* Read compare.c for more information */

#ifndef SOLEMN_COMPARE_H
#define SOLEMN_COMPARE_H

/* Flags, bitwise OR them */
#define STR_NOCASE	1	/* Do case-insensitive comparision */
#define STR_MAXLEN	2	/* Don't compare more then ... bytes */
#define STR_WILDCARDS	4	/* Parse wildcard characters * and ? */
#define STR_WILDCARD1	8	/* Parse wildcard characters * and ? in str1 */
#define STR_WILDCARD2	16	/* Parse wildcard characters * and ? in str2 */

/* Compare str1 and str2
 * Returns 1 on match, 0 on mismatch
*/
int str_compare(char const*, char const*, int, ...);

#endif /* !SOLEMN_COMPARE_H */
