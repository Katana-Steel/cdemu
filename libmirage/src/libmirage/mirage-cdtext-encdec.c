/*
 *  libMirage: CD-TEXT Encoder/Decoder object
 *  Copyright (C) 2006-2009 Rok Mandeljc
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
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mirage.h"

#define __debug__ "CDTEXT-EncDec"


/******************************************************************************\
 *                             Common definitions                             *
\******************************************************************************/
typedef struct {
    /* Header */
    guint8 pack_type;
    guint8 track_number;
    guint8 seq_number;
	guint8 block_number;
    /* Data */
    guint8 data[12];
    /* CRC */
    guint8 crc[2];
} CDTextPack;

typedef struct {
    guint8 charset;
	guint8 first_track;
	guint8 last_track;
	guint8 copyright;
	guint8 pack_count[16];
	guint8 last_seqnum[8];
	guint8 language_codes[8];
} CDTextSizeInfo;

static guint16 cdtext_crc_lut[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
    0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
    0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
    0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
    0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
    0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
    0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
    0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
    0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
    0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
    0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
    0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
    0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
    0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
    0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
    0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
    0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
    0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
    0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
    0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
    0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
    0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


/******************************************************************************\
 *                              Private structure                             *
\******************************************************************************/
#define MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), MIRAGE_TYPE_CDTEXT_ENCDEC, MIRAGE_CDTextEncDecPrivate))

typedef struct {
    gint langcode; /* Language codes */
    gint charset; /* Character set */
    gint first_track; /* First track */
    gint last_track; /* Last track */
    gint copyright; /* Copyright flags */
    
    GList *packs_list; /* List of packs data */
        
    CDTextPack *size_info; /* Pointer to size info packs */
    gint seq_count; /* Sequence count */
    gint pack_count[16]; /* Pack types count */
} MIRAGE_CDTextEncDecBlock;

typedef struct {   
    /* Buffer */
    guint8 *buffer;
    gint buflen;
    
    /* EncDec */
    CDTextPack *cur_pack; /* Pointer to current pack */
    gint cur_pack_fill; /* How much of current pack is used */
    
    /* Blocks */
    gint num_blocks; /* Number of blocks */
    MIRAGE_CDTextEncDecBlock *blocks;
    
    gint length; /* Overall length, in packs */
} MIRAGE_CDTextEncDecPrivate;


/******************************************************************************\
 *                              Private functions                             *
\******************************************************************************/
static GValueArray *__create_value_array (gint block, gint type, gint track, guint8 *data, gint data_len) {
    GValueArray *pack_data = g_value_array_new(5);
    
    /* Internal representation of pack: block, type, track, data, length */
    g_value_array_append(pack_data, NULL);
    g_value_init(g_value_array_get_nth(pack_data, 0), G_TYPE_INT);
    g_value_set_int(g_value_array_get_nth(pack_data, 0), block);
    
    g_value_array_append(pack_data, NULL);
    g_value_init(g_value_array_get_nth(pack_data, 1), G_TYPE_INT);
    g_value_set_int(g_value_array_get_nth(pack_data, 1), type);
    
    g_value_array_append(pack_data, NULL);
    g_value_init(g_value_array_get_nth(pack_data, 2), G_TYPE_INT);
    g_value_set_int(g_value_array_get_nth(pack_data, 2), track);
    
    g_value_array_append(pack_data, NULL);
    g_value_init(g_value_array_get_nth(pack_data, 3), G_TYPE_POINTER);
    g_value_set_pointer(g_value_array_get_nth(pack_data, 3), g_memdup(data, data_len));
    
    g_value_array_append(pack_data, NULL);
    g_value_init(g_value_array_get_nth(pack_data, 4), G_TYPE_INT);
    g_value_set_int(g_value_array_get_nth(pack_data, 4), data_len);
    
    return pack_data;
}

static gint __sort_pack_data (GValueArray *pack1, GValueArray *pack2) {   
    gint block1 = g_value_get_int(g_value_array_get_nth(pack1, 0));
    gint block2 = g_value_get_int(g_value_array_get_nth(pack2, 0));
    
    if (block1 < block2) {
        return -1;
    } else if (block1 > block2) {
        return 1;
    } else {
        gint type1 = g_value_get_int(g_value_array_get_nth(pack1, 1));
        gint type2 = g_value_get_int(g_value_array_get_nth(pack2, 1));
        
        if (type1 < type2) {
            return -1;
        } else if (type1 > type2) {
            return 1;
        } else {
            gint track1 = g_value_get_int(g_value_array_get_nth(pack1, 2));
            gint track2 = g_value_get_int(g_value_array_get_nth(pack2, 2));
            
            if (track1 < track2) {
                return -1;
            } else if (track1 > track2) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}

static gboolean __mirage_cdtext_encdec_cleanup (MIRAGE_CDTextEncDec *self) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    gint i;
    
    /* Cleanup the lists */
    for (i = 0; i < _priv->num_blocks; i++) {
        GList *list = _priv->blocks[i].packs_list;
        if (list) {
            GList *entry = NULL;
            G_LIST_FOR_EACH(entry, list) {
                /* Free pack data */
                g_free(g_value_get_pointer(g_value_array_get_nth(entry->data, 3)));
                /* Free pack */
                g_value_array_free(entry->data);
            }
            g_list_free(list);
        }
    }
    
    _priv->buffer = NULL;
    _priv->buflen = 0;
    
    memset(_priv->blocks, 0, _priv->num_blocks*sizeof(MIRAGE_CDTextEncDecBlock));
    
    return TRUE;
}

static gboolean __mirage_cdtext_encdec_lang2block (MIRAGE_CDTextEncDec *self, gint langcode, gint *block, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    gint ret = 0;
    gint i;
    
    for (i = 0; i < 8; i++) {
        if (_priv->blocks[i].langcode == langcode) {
            ret = i;
            break;
        }
    }
    
    *block = ret;
    return TRUE;
}

static gboolean __mirage_cdtext_encdec_block2lang (MIRAGE_CDTextEncDec *self, gint block, gint *langcode, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    *langcode = _priv->blocks[block].langcode;
    return TRUE;
}

static gboolean __mirage_cdtext_encoder_add_crc_to_pack (MIRAGE_CDTextEncDec *self, CDTextPack *pack, GError **error) {
    /* Calculate CRC for given pack */
    guint8 *data = (guint8 *)pack;
    guint16 crc = 0;
    gint i;
    
    for (i = 0; i < 16; i++) {
        crc = cdtext_crc_lut[(crc >> 8) ^ data[i]] ^ (crc << 8);
    }
    
    crc = ~crc;
    
    pack->crc[0] = (crc & 0xFF00) >> 8;
    pack->crc[1] = crc & 0x00FF;
    
    return TRUE;
}

static gboolean __mirage_cdtext_encoder_initialize_pack (MIRAGE_CDTextEncDec *self, gint block, gint type, gint track, gint carry_len) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    if (!_priv->cur_pack->pack_type) {
        /*g_debug("%s: Empty pack, initializing\n", __debug__);*/
        _priv->cur_pack->pack_type = type;
        if (type != MIRAGE_LANGUAGE_PACK_SIZE) {
            _priv->cur_pack->track_number = track;
            _priv->cur_pack->seq_number = _priv->blocks[block].seq_count;
            _priv->cur_pack->block_number |= (block) << 4;
            _priv->cur_pack->block_number |= (carry_len < 15) ? carry_len : 15;
        } else {
            /* Special handling for 0x8F packs */
            _priv->cur_pack->track_number = _priv->blocks[block].pack_count[type-0x80];
            _priv->cur_pack->seq_number = _priv->blocks[block].seq_count;
            _priv->cur_pack->block_number |= (block) << 4;
            /* Set up pointer to first pack of size info */
            if (!_priv->blocks[block].size_info) {
                _priv->blocks[block].size_info = _priv->cur_pack;
            }
        }
        
        _priv->blocks[block].seq_count++;
        _priv->blocks[block].pack_count[type-0x80]++;
        _priv->length++;
    } else {
        /*g_debug("%s: Pack already initialized (0x%X)\n", __debug__, encoder->cur_pack->pack_type);*/
    }
    
    return TRUE;
}

static gboolean __mirage_cdtext_encoder_pack_data (MIRAGE_CDTextEncDec *self, GValueArray *pack, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    /* Unpack data */
    gint block_number = g_value_get_int(g_value_array_get_nth(pack, 0));
    gint pack_type = g_value_get_int(g_value_array_get_nth(pack, 1));
    gint track_number = g_value_get_int(g_value_array_get_nth(pack, 2));
    guint8 *data = g_value_get_pointer(g_value_array_get_nth(pack, 3));
    gint len = g_value_get_int(g_value_array_get_nth(pack, 4));
    
    /* If current pack is already initialized and the data we're trying to pack
       is if different type, open new pack; this way, we don't have to check if 
       the language has changed (if it has, so has the pack type... from 0x8F 
       to 0x8X */
    if (_priv->cur_pack->pack_type && (pack_type != _priv->cur_pack->pack_type)) {
        /*g_debug("%s: Different pack type, open new pack!\n", __debug__);*/
        _priv->cur_pack++;
        _priv->cur_pack_fill = 0;
    }
       
    gint cur_len = len;
    guint8 *ptr = data;
    gint carry_len = 0; /* How many of characters are contained in previous pack(s) */
    while (cur_len > 0) {
        /* If the current pack is full, open a new one */
        if (_priv->cur_pack_fill == 12) {
            /*g_debug("%s: Current pack full, moving to next!\n", __debug__);*/
            /* Open new pack */
            _priv->cur_pack++;
            _priv->cur_pack_fill = 0;
        }
        
        __mirage_cdtext_encoder_initialize_pack(self, block_number, pack_type, track_number, carry_len);
        
        gint copy_len = MIN(12 - _priv->cur_pack_fill, cur_len);
        /*g_debug("%s: Copying %i bytes (cur_len = %i, cur_fill = %i)\n", __debug__, copy_len, cur_len, encoder->cur_pack_fill);*/
        
        memcpy(_priv->cur_pack->data+_priv->cur_pack_fill, ptr, copy_len);
        
        /* Set the current fill */
        _priv->cur_pack_fill += copy_len;
        
        cur_len -= copy_len;
        ptr += copy_len;
        carry_len += copy_len;
    }
    
    return TRUE;
}

static gboolean __mirage_cdtext_encoder_generate_size_info (MIRAGE_CDTextEncDec *self, gint block, guint8 **data, gint *len, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);

    CDTextSizeInfo *size_info = g_new0(CDTextSizeInfo, 1);
    gint i;
    
    size_info->charset = _priv->blocks[block].charset; /* Character set */
    size_info->first_track = _priv->blocks[block].first_track; /* First track */
    size_info->last_track = _priv->blocks[block].last_track; /* Last track */
    size_info->copyright = _priv->blocks[block].copyright; /* Copyright */
    
    /* Pack count */
    for (i = 0; i < 16; i++) {
        size_info->pack_count[i] = _priv->blocks[block].pack_count[i];
    }
    
    /* Last sequence numbers and language codes */
    for (i = 0; i < 8; i++) {
        /* Set only if we have at least one pack for that language */
        if (_priv->blocks[i].seq_count > 0) {
            size_info->last_seqnum[i] = _priv->blocks[i].seq_count-1; /* It overshoots by one */
            size_info->language_codes[i] = _priv->blocks[i].langcode;
        }
    }
    
    *data = (guint8 *)size_info;
    *len = sizeof(CDTextSizeInfo);
    
    return TRUE;
}

static gboolean __mirage_cdtext_decoder_read_size_info (MIRAGE_CDTextEncDec *self, CDTextPack *size_info_pack, CDTextSizeInfo **data, GError **error) {
    guint8 *size_info = g_malloc0(sizeof(CDTextSizeInfo));
    CDTextPack *cur_pack = size_info_pack;
    gint i;
    
    for (i = 0; i < sizeof(CDTextSizeInfo)/12; i++) {
        memcpy(size_info+12*i, cur_pack->data, 12);
        cur_pack++;
    }
    
    *data = (CDTextSizeInfo *)size_info;
    return TRUE;
}

/******************************************************************************\
 *                             Public API: Encoder                            *
\******************************************************************************/
/**
 * mirage_cdtext_encoder_init:
 * @self: a #MIRAGE_CDTextEncDec
 * @buffer: buffer into which data will be encoded
 * @buflen: buffer length
 * @error: location to store error, or %NULL
 * 
 * <para>
 * Initializes CD-TEXT encoder.
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_encoder_init (MIRAGE_CDTextEncDec *self, guint8 *buffer, gint buflen, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    MIRAGE_CHECK_ARG(buffer);
    
    /* Cleanup old data */
    __mirage_cdtext_encdec_cleanup(self);
    
    /* Set new buffer */
    _priv->buffer = buffer;
    _priv->buflen = buflen;
    
    _priv->cur_pack = (CDTextPack *)_priv->buffer;
        
    return TRUE;
}

/**
 * mirage_cdtext_encoder_set_block_info:
 * @self: a #MIRAGE_CDTextEncDec
 * @block: block number
 * @langcode: language code
 * @charset: character set
 * @copyright: copyright flag
 * @error: location to store error, or %NULL
 *
 * <para>
 * Sets block information for CD-TEXT block specified by @block. @block must be 
 * a valid block number (0-7). @langcode is the language code that is to be assigned 
 * to the block (e.g. 9 for English), @charset denotes character set that is used within
 * the block, and @copyright is the copyright flag for the block.
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_encoder_set_block_info (MIRAGE_CDTextEncDec *self, gint block, gint langcode, gint charset, gint copyright, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    /* Verify that block is valid */
    if (block > _priv->num_blocks) {
        MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: invalid block (%i)!\n", __debug__, block);
        mirage_error(MIRAGE_E_INVALIDARG, error);
        return FALSE;
    }
    
    MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: initialized block %i; langcode: %i; charset: %i; copyright: %i\n", __debug__, block, langcode, charset, copyright);
    _priv->blocks[block].langcode = langcode;
    _priv->blocks[block].charset = charset;
    _priv->blocks[block].copyright = copyright;
    
    return TRUE;
}

/**
 * mirage_cdtext_encoder_add_data:
 * @self: a #MIRAGE_CDTextEncDec
 * @langcode: language code
 * @type: data type
 * @track: track number
 * @data: data
 * @data_len: data length
 * @error: location to store error, or %NULL
 *
 * <para>
 * Adds data to the encoder. @langcode is language code of the block the data
 * should be added to. @type denotes pack type and should be one of #MIRAGE_Language_PackTypes.
 * @track is track number the data belongs to, or 0 if data is global (belongs to disc/session).
 * @data is buffer containing data to be added, and @data_len is length of data in the buffer.
 * </para>
 * 
 * <para>
 * This function does not perform any encoding yet; it merely adds the data into 
 * encoder's internal representation of CD-TEXT block.
 * </para>
 * 
 * <note>
 * Block needs to have its information set with mirage_cdtext_encoder_set_block_info() 
 * before data can be added to it.
 * </note>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_encoder_add_data (MIRAGE_CDTextEncDec *self, gint langcode, gint type, gint track, guint8 *data, gint data_len, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    GValueArray *pack_data = NULL;
    
    /* Langcode -> block conversion */
    gint block = 0;
    if (!__mirage_cdtext_encdec_lang2block(self, langcode, &block, error)) {
        return FALSE;
    }
    
    pack_data = __create_value_array(block, type, track, data, data_len);    
    
    /* Add internal representation to ordered list... */
    _priv->blocks[block].packs_list = g_list_insert_sorted(_priv->blocks[block].packs_list, pack_data, (GCompareFunc)__sort_pack_data);
    
    /* Set the number of first track that has language... this is not
       very reliable, but I do believe all tracks from now on are
       required by specs to have language block? */
    if (!_priv->blocks[block].first_track) {
        _priv->blocks[block].first_track = track;
    }
    /* Keep setting the last track... */
    _priv->blocks[block].last_track = track;
    
    return TRUE;
}

/**
 * mirage_cdtext_encoder_encode:
 * @self: a #MIRAGE_CDTextEncDec
 * @buffer: location to store buffer
 * @buflen: location to store buffer length
 * @error: location to store error, or %NULL
 *
 * <para>
 * Encodes the CD-TEXT data. Pointer to buffer containing the encoded data is
 * stored in @buffer, and length of data in buffer is stored in @buflen.
 * </para>
 *
 * <para>
 * Note that @buffer is the same as the argument passed to mirage_cdtext_encoder_init().
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_encoder_encode (MIRAGE_CDTextEncDec *self, guint8 **buffer, gint *buflen, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    gint i;

    MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: encoding CD-TEXT...\n", __debug__);
    
    /* Encode all blocks */
    for (i = 0; i < _priv->num_blocks; i++) {
        /* Block is valid only if it has langcode set */
        if (_priv->blocks[i].langcode) {
            GList *entry = NULL;
            
            MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: encoding block %i; langcode %i\n", __debug__, i, _priv->blocks[i].langcode);
            
            /* Encode all on list */
            G_LIST_FOR_EACH(entry, _priv->blocks[i].packs_list) {
                GValueArray *pack_data = entry->data;
                __mirage_cdtext_encoder_pack_data(self, pack_data, NULL);
            }
            
            /* We need to 'reserve' some space for size info */
            CDTextSizeInfo size_info = {0};
            GValueArray *dummy_data = __create_value_array(i, 0x8F, 0, (guint8 *)&size_info, sizeof(size_info));
            __mirage_cdtext_encoder_pack_data(self, dummy_data, NULL);
            g_value_array_free(dummy_data);
        } else {
            MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: block %i not valid\n", __debug__, i);
        }
    }
    
    /* Now that all the packs have been encoded, we can generate size info */
    for (i = 0; i < 8; i++) {
        /* Having pointer set means we're using this language... */
        if (_priv->blocks[i].size_info) {
            guint8 *size_info = NULL;
            gint size_info_len = 0;
            
            __mirage_cdtext_encoder_generate_size_info(self, i, &size_info, &size_info_len, NULL);
            
            /* Write into previously reserved size info packs */
            /*g_debug("%s: Writing info into packs at %p\n", __debug__, encoder.size_info[i]);*/
            _priv->cur_pack = _priv->blocks[i].size_info;
            _priv->cur_pack_fill = 0;
            gint old_len = _priv->length;
            
            GValueArray *pack_data = __create_value_array(i, 0x8F, 0, (guint8 *)size_info, size_info_len);
            __mirage_cdtext_encoder_pack_data(self, pack_data, NULL);
            _priv->length = old_len;
            
            g_value_array_free(pack_data);
            g_free(size_info);
        }
    }
    
    /* Generate CRC for all packs */
    _priv->cur_pack = (CDTextPack *)_priv->buffer;
    for (i = 0; i < _priv->length; i++) {
        __mirage_cdtext_encoder_add_crc_to_pack(self, _priv->cur_pack, NULL);
        _priv->cur_pack++;
    }    
            
    *buflen = _priv->length*sizeof(CDTextPack);
    *buffer = _priv->buffer;

    MIRAGE_DEBUG(self, MIRAGE_DEBUG_CDTEXT, "%s: done encoding CD-TEXT; length: 0x%X\n", __debug__, *buflen);
    
    return TRUE;
}


/******************************************************************************\
 *                             Public API: Decoder                            *
\******************************************************************************/
/**
 * mirage_cdtext_decoder_init:
 * @self: a #MIRAGE_CDTextEncDec
 * @buffer: buffer containing encoded data
 * @buflen: length of data in buffer
 * @error: location to store error, or %NULL
 *
 * <para>
 * Initializes CD-TEXT decoder. @buffer is the buffer containing encoded CD-TEXT
 * data and @buflen is length of data in the buffer.
 * </para>
 *
 * <para>
 * This function decodes CD-TEXT data and stores it in decoder's internal representation.
 * Information about decoded CD-TEXT blocks and their data can be obtained via 
 * subsequent calls to mirage_cdtext_decoder_get_block_info() and 
 * mirage_cdtext_decoder_get_data().
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_decoder_init (MIRAGE_CDTextEncDec *self, guint8 *buffer, gint buflen, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    /* Cleanup old data */
    __mirage_cdtext_encdec_cleanup(self);
    
    /* Set new buffer */
    _priv->buffer = buffer;
    _priv->buflen = buflen;
    _priv->cur_pack = (CDTextPack *)_priv->buffer;
    _priv->length = buflen/sizeof(CDTextPack);
        
    /* Read size info packs */
    gint i = 0;
    while (i < _priv->length) {
        CDTextPack *cur_pack = _priv->cur_pack+i;
        
        if (cur_pack->pack_type == 0x8F) {
            gint block = (cur_pack->block_number & 0xF0) >> 4;
            _priv->blocks[block].size_info = cur_pack;
            
            CDTextSizeInfo *size_info = NULL;
            if (__mirage_cdtext_decoder_read_size_info(self, cur_pack, &size_info, NULL)) {
                _priv->blocks[block].langcode = size_info->language_codes[block];
                _priv->blocks[block].charset = size_info->charset;
                _priv->blocks[block].copyright = size_info->copyright;
                _priv->blocks[block].first_track = size_info->first_track;
                _priv->blocks[block].last_track = size_info->last_track;
                
                _priv->blocks[block].seq_count = size_info->last_seqnum[block] + 1;
                gint j;
                for (j = 0; j < 16; j++) {
                    _priv->blocks[block].pack_count[j] = size_info->pack_count[j];
                }
                g_free(size_info);
            }
            
            i += 3;
        } else {
            i++;
        }
    }
    
    /* Now decode... go over all blocks and decode valid ones */
    gint block;
    for (block = 0; block < 8; block++) {
        gchar tmp_buffer[255];
        memset(tmp_buffer, 0xFF, sizeof(tmp_buffer));
        gchar *ptr = tmp_buffer;
        gint tmp_len = 0;
        gint cur_track = 0;
        
        /* Skip empty blocks */
        if (!_priv->blocks[block].seq_count) {
            continue;
        }
        
        _priv->cur_pack = _priv->blocks[block].size_info - (_priv->blocks[block].seq_count - 3);
        
        while (_priv->cur_pack < _priv->blocks[block].size_info) {
            if (_priv->cur_pack->pack_type != (_priv->cur_pack - 1)->pack_type) {
                /*g_debug("%s: new pack type, resetting strings...\n", __debug__);*/
                memset(tmp_buffer, 0xFF, sizeof(tmp_buffer));
                tmp_len = 0;
                ptr = tmp_buffer;
            }
            
            /* Current data offset */
            gchar *cur_data = (gchar *)_priv->cur_pack->data + _priv->cur_pack_fill;
            /* Length of transfer */
            gint copy_len = MIN(strlen(cur_data) + 1, 12 - _priv->cur_pack_fill);
            
            /* Copy */
            memcpy(ptr, _priv->cur_pack->data + _priv->cur_pack_fill, copy_len);
            /* Do some necesssary calcs */
            _priv->cur_pack_fill += copy_len;
            ptr += copy_len;
            tmp_len += copy_len;
            
            /* The way of the lazy programmer (TM)... we set whole string to 0xFF
               and then wait 'till we get terminating 0 at the end :D (we also need
               to check whether string doesn't contain just "\0", which keeps getting
               returned once we hit the padding at the end of a pack types...) */
            if (!tmp_buffer[tmp_len-1] && strlen(tmp_buffer)) {
                /*g_debug("%s: block: %i; pack type: 0x%X; track: %i; len: %i; data: %s\n", __debug__, block, _priv->cur_pack->pack_type, cur_track, tmp_len, tmp_buffer);*/
                
                /* Pack the data and add it to the list; as simple as that... */
                GValueArray *pack_data = __create_value_array(block, _priv->cur_pack->pack_type, cur_track, g_memdup(tmp_buffer, tmp_len), tmp_len); 
                _priv->blocks[block].packs_list = g_list_insert_sorted(_priv->blocks[block].packs_list, pack_data, (GCompareFunc)__sort_pack_data);

                /* Clear the temporary buffer */
                memset(tmp_buffer, 0xFF, sizeof(tmp_buffer));
                tmp_len = 0;
                ptr = tmp_buffer;
                
                /* Increase track number; this is to account for strings too short
                   to cause switch to next pack, whose track number is consequently
                   stored nowhere... */
                cur_track++;
            }

            if (_priv->cur_pack_fill == 12) {
                /*g_debug("%s: reached the end of packet, going to the next one\n");*/
                _priv->cur_pack_fill = 0;
                _priv->cur_pack++;
                /* Set current track number */
                cur_track = _priv->cur_pack->track_number;
            }
        }
    }
    
    return TRUE;
}

/**
 * mirage_cdtext_decoder_get_block_info:
 * @self: a #MIRAGE_CDTextEncDec
 * @block: block number
 * @langcode: location to store language code
 * @charset: location to store character set
 * @copyright: location to store copyright flag
 * @error: location to store error, or %NULL
 *
 * <para>
 * Retrieves block information for CD-TEXT block specified by @block. @block 
 * must be a valid block number (0-7). Language code assigned to the block is
 * stored in @langcode, code of character set used within block is stored in
 * @charset and block's copyright flag is stored in @copyright.
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_decoder_get_block_info (MIRAGE_CDTextEncDec *self, gint block, gint *langcode, gint *charset, gint *copyright, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    /* Verify that block is valid */
    if (block > _priv->num_blocks) {
        mirage_error(MIRAGE_E_INVALIDARG, error);
        return FALSE;
    }
    if (!_priv->blocks[block].langcode) {
        /* FIXME: error */
        mirage_error(MIRAGE_E_GENERIC, error);
        return FALSE;
    }
    
    if (langcode) {
        *langcode = _priv->blocks[block].langcode;
    }
    if (charset) {
        *charset = _priv->blocks[block].charset;
    }
    if (copyright) {
        *copyright = _priv->blocks[block].copyright;
    }
    
    return TRUE;
}

/**
 * mirage_cdtext_decoder_get_data:
 * @self: a #MIRAGE_CDTextEncDec
 * @block: block number
 * @callback_func: callback function
 * @user_data: data to be passed to callback function
 * @error: location to store error, or %NULL
 *
 * <para>
 * Retrieves data for CD-TEXT block specified by @block. @block must be a valid 
 * block number (0-7). It calls @callback_func for every data pack that has been
 * encoded in the block.
 * </para>
 *
 * <para>
 * If @callback_func returns %FALSE, the function immediately returns %FALSE and
 * @error is set to %MIRAGE_E_ITERCANCELLED.
 * </para>
 *
 * Returns: %TRUE on success, %FALSE on failure
 **/
gboolean mirage_cdtext_decoder_get_data (MIRAGE_CDTextEncDec *self, gint block, MIRAGE_CDTextDataCallback callback_func, gpointer user_data, GError **error) {
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    GList *entry = NULL;
    
    /* Go over the list and call the callback for each entry */
    G_LIST_FOR_EACH(entry, _priv->blocks[block].packs_list) {
        GValueArray *pack_data = entry->data;
        
        gint block_number = g_value_get_int(g_value_array_get_nth(pack_data, 0));
        gint pack_type = g_value_get_int(g_value_array_get_nth(pack_data, 1));
        gint track_number = g_value_get_int(g_value_array_get_nth(pack_data, 2));
        guint8 *data = g_value_get_pointer(g_value_array_get_nth(pack_data, 3));
        gint len = g_value_get_int(g_value_array_get_nth(pack_data, 4));
        
        gint langcode = 0;
        __mirage_cdtext_encdec_block2lang(self, block_number, &langcode, NULL);
        
        if (!callback_func(langcode, pack_type, track_number, data, len, user_data)) {
            mirage_error(MIRAGE_E_ITERCANCELLED, error);
            return FALSE;
        }
    }
    
    return TRUE;
}


/******************************************************************************\
 *                                 Object init                                *
\******************************************************************************/
/* Our parent class */
static MIRAGE_ObjectClass *parent_class = NULL;

static void __mirage_cdtext_encdec_instance_init (GTypeInstance *instance, gpointer g_class) {
    MIRAGE_CDTextEncDec *self = MIRAGE_CDTEXT_ENCDEC(instance);
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);
    
    /* Specs say there can be 8 blocks max... */
    _priv->num_blocks = 8;
    _priv->blocks = g_new0(MIRAGE_CDTextEncDecBlock, _priv->num_blocks);
    
    return;
}

static void __mirage_cdtext_encdec_finalize (GObject *obj) {
    MIRAGE_CDTextEncDec *self = MIRAGE_CDTEXT_ENCDEC(obj);
    MIRAGE_CDTextEncDecPrivate *_priv = MIRAGE_CDTEXT_ENCDEC_GET_PRIVATE(self);

    MIRAGE_DEBUG(self, MIRAGE_DEBUG_GOBJECT, "%s: finalizing object\n", __debug__);
    
    /* Cleanup the data */
    __mirage_cdtext_encdec_cleanup(self);
    
    /* Free blocks data */
    g_free(_priv->blocks);
        
    /* Chain up to the parent class */
    MIRAGE_DEBUG(self, MIRAGE_DEBUG_GOBJECT, "%s: chaining up to parent\n", __debug__);
    return G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void __mirage_cdtext_encdec_class_init (gpointer g_class, gpointer g_class_data) {
    GObjectClass *class_gobject = G_OBJECT_CLASS(g_class);
    MIRAGE_CDTextEncDecClass *klass = MIRAGE_CDTEXT_ENCDEC_CLASS(g_class);
    
    /* Set parent class */
    parent_class = g_type_class_peek_parent(klass);
    
    /* Register private structure */
    g_type_class_add_private(klass, sizeof(MIRAGE_CDTextEncDecPrivate));
    
    /* Initialize GObject methods */
    class_gobject->finalize = __mirage_cdtext_encdec_finalize;
    
    return;
}

GType mirage_cdtext_encdec_get_type (void) {
    static GType type = 0;
    if (type == 0) {
        static const GTypeInfo info = {
            sizeof(MIRAGE_CDTextEncDecClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            __mirage_cdtext_encdec_class_init,   /* class_init */
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(MIRAGE_CDTextEncDec),
            0,      /* n_preallocs */
            __mirage_cdtext_encdec_instance_init    /* instance_init */
        };
        
        type = g_type_register_static(MIRAGE_TYPE_OBJECT, "MIRAGE_CDTextEncDec", &info, 0);
    }
    
    return type;
}