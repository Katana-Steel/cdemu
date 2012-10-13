/*
 *  libMirage: Debugging facilities
 *  Copyright (C) 2006-2012 Rok Mandeljc
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MIRAGE_DEBUG_H__
#define __MIRAGE_DEBUG_H__


/**
 * MirageDebugMasks:
 * @MIRAGE_DEBUG_ERROR: error message
 * @MIRAGE_DEBUG_WARNING: warning message
 * @MIRAGE_DEBUG_PARSER: message belonging to disc parser and file stream parser
 * @MIRAGE_DEBUG_DISC: message belonging to disc
 * @MIRAGE_DEBUG_SESSION: message belonging to session
 * @MIRAGE_DEBUG_TRACK: message belonging to track
 * @MIRAGE_DEBUG_SECTOR: message belonging to sector
 * @MIRAGE_DEBUG_FRAGMENT: message belonging to fragment
 * @MIRAGE_DEBUG_CDTEXT: message belonging to CD-TEXT encoder/decoder
 * @MIRAGE_DEBUG_FILE_IO: messages belonging to file filter I/O operations
 *
 * <para>
 * Debug message types and debug masks used to control verbosity of various
 * parts of libMirage.
 * </para>
 *
 * <para>
 * All masks except %MIRAGE_DEBUG_ERROR and %MIRAGE_DEBUG_WARNING can be combined
 * together to control verbosity of libMirage.
 * </para>
 **/
typedef enum _MirageDebugMasks
{
    /* Debug types */
    MIRAGE_DEBUG_ERROR    = -1,
    MIRAGE_DEBUG_WARNING  = -2,
    /* Debug masks */
    MIRAGE_DEBUG_PARSER  = 0x1,
    MIRAGE_DEBUG_DISC = 0x2,
    MIRAGE_DEBUG_SESSION = 0x4,
    MIRAGE_DEBUG_TRACK = 0x8,
    MIRAGE_DEBUG_SECTOR = 0x10,
    MIRAGE_DEBUG_FRAGMENT = 0x20,
    MIRAGE_DEBUG_CDTEXT = 0x40,
    MIRAGE_DEBUG_FILE_IO = 0x80,
} MirageDebugMasks;


/**
 * MIRAGE_DEBUG:
 * @obj: (in): object
 * @lvl: (in): debug level
 * @msg...: (in): debug message
 *
 * <para>
 * Debugging macro, provided for convenience. It performs cast to
 * #MirageContextual interface on @obj and calls mirage_contextual_debug_message()
 * with debug level @lvl and debug message @msg....
 * </para>
 **/
#define MIRAGE_DEBUG(obj, lvl, ...) mirage_contextual_debug_message(MIRAGE_CONTEXTUAL(obj), lvl, __VA_ARGS__)


#endif /* __MIRAGE_DEBUG_H__ */
