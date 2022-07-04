/*
  libcon - run with unspecified MCS category

  Copyright (C) 2022 Mark Williams, Jr.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License along
  with this library; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include <selinux/selinux.h>
#include <selinux/context.h>

#define ENV_NAME "LD_PRELOAD"
#define LIB_NAME "libcon.so"

static void
child_cb (void)
{
  char *str;

  if (!getcon (&str) && str)
    {
      context_t context = context_new (str);
      const char *s = context_range_get (context);

      if (s)
        {
          char range[strlen (s) + 7];

          if (sprintf (strchrnul (strcpy (range, s), ':'),
                       ":c%u",
                       ((struct { unsigned n:10; }) { clock () }).n) > 2 &&
              !context_range_set (context, range))
            {
              char *value = getenv(ENV_NAME);

              if (value)
                {
                  char *str = strstr (value, LIB_NAME);

                  if (str)
                    {
                      char *dest, *src;

                      while ((str = strstr (src += strspn(src = (dest = str) + 9, " :"),
                                            LIB_NAME)))
                        strncpy(dest, src, str - src);

                      strcpy(dest, src);
                    }

                  if (value[0])
                    setenv (ENV_NAME, value, 1);
                  else
                    unsetenv (ENV_NAME);
                }

              setcon (context_str (context));
            }

          memset (range, 0, sizeof range);
        }

      context_free (context);
    }

  freecon (str);
}

static void __attribute__ ((constructor))
libcon (void)
{
  pthread_atfork (NULL, NULL, child_cb);
}
