#include "kle_reader/kle_reader.h"
#include "xjsmn/x_jsmn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KLE_STRING_MAX_LENGTH 31
#define KLE_LABEL_MAX 12
#define KLE_KEY_MAX 256

namespace kle
{
    static int to_digit(char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return 0;
    }

    static char to_lower(char c)
    {
        if (c >= 'A' && c <= 'Z')
            return c - 'A' + 'a';
        return c;
    }

    color_t str_t::to_color(const char* json) const
    {
        color_t c;
        if (m_begin == -1 || m_end == -1)
            return c;

        const char* str = json + m_begin;

        // parse color
        if (str[0] == '#')
        {
            if ((m_end - m_begin) >= 7)
            {
                c.m_r = to_digit(str[1]) * 16 + to_digit(str[2]);
                c.m_g = to_digit(str[3]) * 16 + to_digit(str[4]);
                c.m_b = to_digit(str[5]) * 16 + to_digit(str[6]);
                c.m_a = 255;
                if ((m_end - m_begin) == 9)
                {
                    c.m_a = to_digit(str[7]) * 16 + to_digit(str[8]);
                }
            }
        }
        return c;
    }

    int str_t::to_int(const char* json) const
    {
        if (m_begin == -1 || m_end == -1)
            return 0;
        const char* str  = json + m_begin;
        const char* end  = json + m_end;
        int         val  = 0;
        int         sign = 1;
        if (*str == '-')
        {
            sign = -1;
            str++;
        }
        while (str < end)
        {
            val = val * 10 + (*str - '0');
            str++;
        }
        return val * sign;
    }

    bool str_t::to_bool(const char* json) const
    {
        if (m_begin == -1 || m_end == -1)
            return 0;

        const char* str = json + m_begin;
        const int   len = m_end - m_begin;
        bool        val = false;
        if (len == 4 && to_lower(str[0]) == 't' && to_lower(str[1]) == 'r' && to_lower(str[2]) == 'u' && to_lower(str[3]) == 'e')
            val = true;
        return val;
    }

    static void init(key_t& key)
    {
        key.m_color = color_t(0xcc, 0xcc, 0xcc);
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            key.m_labels[i]    = str_t();
            key.m_textColor[i] = color_t(0x00, 0x00, 0x00);
            key.m_textSize[i]  = 3;
        }
        key.m_default_textColor = color_t();
        key.m_default_textSize  = 0;
        key.m_x                 = 0;
        key.m_y                 = 0;
        key.m_width             = 1;
        key.m_height            = 1;
        key.m_x2                = 0;
        key.m_y2                = 0;
        key.m_width2            = 1;
        key.m_height2           = 1;
        key.m_rotation_x        = 0;
        key.m_rotation_y        = 0;
        key.m_rotation_angle    = 0;
        key.m_decal             = false;
        key.m_ghost             = false;
        key.m_stepped           = false;
        key.m_nub               = false;
        key.m_profile           = str_t();
    }

    static key_t copy(key_t const& key)
    {
        key_t result;
        result.m_color = key.m_color;
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            result.m_labels[i]    = key.m_labels[i];
            result.m_textColor[i] = key.m_textColor[i];
            result.m_textSize[i]  = key.m_textSize[i];
        }
        result.m_default_textColor = key.m_default_textColor;
        result.m_default_textSize  = key.m_default_textSize;
        result.m_x                 = key.m_x;
        result.m_y                 = key.m_y;
        result.m_width             = key.m_width;
        result.m_height            = key.m_height;
        result.m_x2                = key.m_x2;
        result.m_y2                = key.m_y2;
        result.m_width2            = key.m_width2;
        result.m_height2           = key.m_height2;
        result.m_rotation_x        = key.m_rotation_x;
        result.m_rotation_y        = key.m_rotation_y;
        result.m_rotation_angle    = key.m_rotation_angle;
        result.m_decal             = key.m_decal;
        result.m_ghost             = key.m_ghost;
        result.m_stepped           = key.m_stepped;
        result.m_nub               = key.m_nub;
        result.m_profile           = key.m_profile;
        return result;
    }

    static void init(bkgrnd_t bkgrnd)
    {
        bkgrnd.m_name  = str_t();
        bkgrnd.m_style = str_t();
    }

    static void init(kb_metadata_t& kb_metadata)
    {
        kb_metadata.m_author             = str_t();
        kb_metadata.m_backcolor          = color_t(0xee, 0xee, 0xee);
        kb_metadata.m_background.m_name  = str_t();
        kb_metadata.m_background.m_style = str_t();
        kb_metadata.m_name               = str_t();
        kb_metadata.m_notes              = str_t();
        kb_metadata.m_radii              = str_t();
        kb_metadata.m_switchBrand        = str_t();
        kb_metadata.m_switchMount        = str_t();
        kb_metadata.m_switchType         = str_t();
    }

    static void init(kb_t& kb)
    {
        init(kb.m_meta);
        kb.m_nb_keys = 0;
        for (int i = 0; i < KLE_KEY_MAX; ++i)
        {
            init(kb.m_keys[i]);
        }
    }

    static void add(kb_t& kb, key_t const& key)
    {
        kb.m_keys[kb.m_nb_keys] = key;
        ++kb.m_nb_keys;
    }

    static int labelMap[8][12] = {
        {0, 6, 2, 8, 9, 11, 3, 5, 1, 4, 7, 10},          /* row 0*/
        {1, 7, -1, -1, 9, 11, 4, -1, -1, -1, -1, 10},    /* row 1*/
        {3, -1, 5, -1, 9, 11, -1, -1, 4, -1, -1, 10},    /* row 2*/
        {4, -1, -1, -1, 9, 11, -1, -1, -1, -1, -1, 10},  /* row 3*/
        {0, 6, 2, 8, 10, -1, 3, 5, 1, 4, 7, -1},         /* row 4*/
        {1, 7, -1, -1, 10, -1, 4, -1, -1, -1, -1, -1},   /* row 5*/
        {3, -1, 5, -1, 10, -1, -1, -1, 4, -1, -1, -1},   /* row 6*/
        {4, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1}, /* row 7*/
    };

    static void reorderLabelsIn(str_t labels[KLE_LABEL_MAX], int align)
    {
        str_t ret[KLE_LABEL_MAX];

		for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
			if (labelMap[align][i]>=0)
			{
                ret[labelMap[align][i]].m_begin = labels[i].m_begin;
                ret[labelMap[align][i]].m_end   = labels[i].m_end;
            }
        }
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            labels[i] = ret[i];
        }
    }

	static void reorderLabelsIn(int sizes[KLE_LABEL_MAX], int align)
	{
		int ret[KLE_LABEL_MAX];
		for (int i = 0; i < KLE_LABEL_MAX; ++i)
			ret[i] = 0;
		
		for (int i = 0; i < KLE_LABEL_MAX; ++i)
		{
			if (labelMap[align][i]>=0)
				ret[labelMap[align][i]] = sizes[i];
		}
		for (int i = 0; i < KLE_LABEL_MAX; ++i)
		{
			sizes[i] = ret[i];
		}
	}


    static void reorderLabelsIn(color_t colors[KLE_LABEL_MAX], int align)
    {
        color_t ret[KLE_LABEL_MAX];

		for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
			if (labelMap[align][i]>=0)
				ret[labelMap[align][i]] = colors[i];
        }
        for (int i = 0; i < KLE_LABEL_MAX; ++i)
        {
            colors[i] = ret[i];
        }
    }

    struct item_t
    {
        str_t a;
        str_t x;
        str_t y;
        str_t c;
        str_t t;
        str_t w;
        str_t h;
        str_t x2;
        str_t y2;
        str_t w2;
        str_t h2;
        str_t rx;
        str_t ry;
        str_t r;
        str_t d;
        str_t g;
        str_t l;
        str_t n;
        str_t p;
        str_t f;
        str_t f2;
        str_t s;
    };

    static int split(const char* json, str_t& item, char split, str_t* items_out, int max_items_out)
    {
        if (item.is_null())
            return 0;

        const char* str = json + item.m_begin;
        const char* end = json + item.m_end;

        // Split the string 'item' into substrings delimited by 'split'
        int nb_items = 0;
        while (str < end)
        {
            const char* next = str;
            while (next < end && *next != split)
            {
                ++next;
            }
            items_out[nb_items].m_begin = (int)(str - json);
            items_out[nb_items].m_end   = (int)(next - json);
            ++nb_items;
            if (nb_items >= max_items_out)
            {
                break;
            }
            str = next + 1;
        }
        return nb_items;
    }

    static bool  key_cmp_str(const char* json, jsmntok_t const& tk, const char* str) { return ((tk.end - tk.start) == strlen(str)) && memcmp(json + tk.start, str, strlen(str)) == 0; }
    static str_t value_to_str(const char* json, jsmntok_t const& tk) { return str_t(tk.start, tk.end); }

    static int parse(const char* json, jsmntok_t* tokens, int index, bkgrnd_t& bg)
    {
        int size = tokens[index].size;

        index++;
        for (int i = 0; i < size; ++i)
        {
            jsmntok_t const& token = tokens[index];
            const char*      key   = json + token.start;
            if (key_cmp_str(json, token, "name"))
            {
                jsmntok_t const& value = tokens[index + 1];
                bg.m_name              = value_to_str(json, value);
            }
            else if (key_cmp_str(json, token, "style"))
            {
                jsmntok_t const& value = tokens[index + 1];
                bg.m_style             = value_to_str(json, value);
            }
            index += 2;
        }
        return index;
    }

    static int parse(const char* json, jsmntok_t* tokens, int index, kb_metadata_t& kbmd)
    {
        int const size = tokens[index].size;
        if (tokens[index].type != JSMN_OBJECT)
        {
            return index + size;
        }

        index++;
        for (int i = 0; i < size; ++i)
        {
            const jsmntok_t& token = tokens[index];
            if (token.type == JSMN_STRING)
            {
                const char* key = json + token.start;

                const jsmntok_t& value = tokens[index + 1];
                if (key_cmp_str(json, token, "author"))
                {
                    kbmd.m_author = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "backcolor"))
                {
                    kbmd.m_backcolor = value_to_str(json, value).to_color(json);
                }
                else if (key_cmp_str(json, token, "background"))
                {
                    index = parse(json, tokens, index + 1, kbmd.m_background);
                    index -= 2;
                }
                else if (key_cmp_str(json, token, "notes"))
                {
                    kbmd.m_notes = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "name"))
                {
                    kbmd.m_name = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "radii"))
                {
                    kbmd.m_radii = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "switchBrand"))
                {
                    kbmd.m_switchBrand = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "switchMount"))
                {
                    kbmd.m_switchMount = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "switchType"))
                {
                    kbmd.m_switchType = value_to_str(json, value);
                }
                index += 2;
            }
        }
        return index;
    }

    static int parse(const char* json, jsmntok_t* tokens, int index, item_t& item)
    {
        // First token should be an object?
        int const size = tokens[index].size;
        if (tokens[index].type != JSMN_OBJECT)
        {
            return index + 1 + size;
        }

        index++;

        for (int i = 0; i < size; ++i)
        {
            const jsmntok_t& token = tokens[index];
            if (token.type == JSMN_STRING)
            {
                const char* key = json + token.start;

                const jsmntok_t& value = tokens[index + 1];
                if (key_cmp_str(json, token, "x"))
                {
                    item.x = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "y"))
                {
                    item.y = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "w"))
                {
                    item.w = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "h"))
                {
                    item.h = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "x2"))
                {
                    item.x2 = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "y2"))
                {
                    item.y2 = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "w2"))
                {
                    item.w2 = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "h2"))
                {
                    item.h2 = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "rx"))
                {
                    item.rx = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "ry"))
                {
                    item.ry = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "r"))
                {
                    item.r = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "a"))
                {
                    item.a = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "d"))
                {
                    item.d = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "g"))
                {
                    item.g = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "l"))
                {
                    item.l = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "n"))
                {
                    item.n = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "p"))
                {
                    item.p = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "f"))
                {
                    item.f = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "f2"))
                {
                    item.f2 = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "fa"))
                {
                    // fa is an array of integers
                    const int fa_size = value.size;
					index += fa_size;
                }
                else if (key_cmp_str(json, token, "c"))
                {
                    item.c = value_to_str(json, value);
                }
                else if (key_cmp_str(json, token, "t"))
                {
                    item.c = value_to_str(json, value);
                }

                index += 2;
            }
            else
            {
                // Unknown item here, try and skip it
                index += 2;
            }
        }
        return index;
    }

    bool read(const char* filename, kb_t& kb)
    {
        // read raw file content
        FILE* pFile;
        fopen_s(&pFile, filename, "rb");
        if (pFile == NULL)
        {
            printf("Error opening file %s\n", filename);
            return false;
        }
        fseek(pFile, 0, SEEK_END);
        long lSize = ftell(pFile) + 16;
        rewind(pFile);
        char* buffer = (char*)malloc(sizeof(char) * lSize);
        memset(buffer, 0, lSize);
        if (buffer == NULL)
        {
            printf("Error allocating memory\n");
            return false;
        }
        fread(buffer, 1, lSize, pFile);
        fclose(pFile);

        // parse json
        const int   max_tokens = 2048;
        jsmntok_t*  tokens     = (jsmntok_t*)malloc(sizeof(jsmntok_t) * max_tokens);
        jsmn_parser parser;
        jsmn_init(&parser, tokens, max_tokens);
        jsmn_strict(&parser, false);
        int num_tokens = jsmn_parse(&parser, buffer, lSize);
        if (num_tokens < 0)
        {
            printf("Failed to parse JSON: %d\n", num_tokens);
            return false;
        }

        // Initialize with defaults
        key_t current;
        init(current);
        init(kb);

        int align = 4;

        for (int r = 1; r < num_tokens;)
        {
            jsmntok_t token = tokens[r];
            if (token.type == JSMN_ARRAY)
            {
                ++r;
                int const n = token.size;
                int       a = 0;
                while (a < n && r < num_tokens)
                {
                    token = tokens[r];
                    if (token.type == JSMN_STRING)
                    {
                        key_t newKey = copy(current);

                        r++;

                        newKey.m_width2  = newKey.m_width2 == 0 ? current.m_width : current.m_width2;
                        newKey.m_height2 = newKey.m_height2 == 0 ? current.m_height : current.m_height2;
						
						split(parser.begin, str_t(token.start, token.end), '\n', newKey.m_labels, 12);
						reorderLabelsIn(newKey.m_labels, align);
						reorderLabelsIn(newKey.m_textSize, align);

                        for (int i = 0; i < 12; ++i)
                        {
                            if (newKey.m_labels[i].is_null())
                            {
                                newKey.m_textSize[i]  = 0;
                                newKey.m_textColor[i] = color_t();
                            }
                        }

                        // Add the key!
                        add(kb, newKey);

                        // Set up for the next key
                        current.m_x += current.m_width;
                        current.m_width = current.m_height = 1;
                        current.m_x2 = current.m_y2 = current.m_width2 = current.m_height2 = 0;
                        current.m_nub = current.m_stepped = current.m_decal = false;
                    }
                    else
                    {
                        item_t item;
                        r = parse(parser.begin, tokens, r, item);
                        if (a != 0 && (!item.r.is_null() || !item.rx.is_null() || !item.ry.is_null()))
                        {
                            // "rotation can only be specified on the first key in a row"
                            return false;
                        }

                        if (!item.r.is_null())
                            current.m_rotation_angle = item.r.to_int(parser.begin);
                        if (!item.rx.is_null())
                            current.m_rotation_x = item.rx.to_int(parser.begin);
                        if (!item.ry.is_null())
                            current.m_rotation_y = item.ry.to_int(parser.begin);

                        if (!item.a.is_null())
                            align = item.a.to_int(parser.begin);

                        if (!item.f.is_null())
                        {
                            current.m_default_textSize = item.f.to_int(parser.begin);
                            for (int i = 0; i < 12; ++i)
                                current.m_textSize[i] = current.m_default_textSize;
                        }
                        if (!item.f2.is_null())
                        {
                            for (int i = 0; i < 12; ++i)
                                current.m_textSize[i] = item.f2.to_int(parser.begin);
                        }

                        if (!item.p.is_null())
                            current.m_profile = item.p;
                        if (!item.c.is_null())
                            current.m_color = item.c.to_color(parser.begin);
                        if (!item.t.is_null())
                        {
                            str_t colors[12];
                            split(parser.begin, item.t, '\n', colors, 12);
                            if (!colors[0].is_null())
                                current.m_default_textColor = colors[0].to_color(parser.begin);
                            reorderLabelsIn(colors, align);
                        }
                        if (!item.x.is_null())
                            current.m_x += item.x.to_int(parser.begin);
                        if (!item.y.is_null())
                            current.m_y += item.y.to_int(parser.begin);
                        if (!item.w.is_null())
                        {
                            current.m_width = current.m_width2 = item.w.to_int(parser.begin);
                        }
                        if (!item.h.is_null())
                        {
                            current.m_height = current.m_height2 = item.h.to_int(parser.begin);
                        }
                        if (!item.x2.is_null())
                            current.m_x2 = item.x2.to_int(parser.begin);
                        if (!item.y2.is_null())
                            current.m_y2 = item.y2.to_int(parser.begin);
                        if (!item.w2.is_null())
                            current.m_width2 = item.w2.to_int(parser.begin);
                        if (!item.h2.is_null())
                            current.m_height2 = item.h2.to_int(parser.begin);
                        if (!item.n.is_null())
                            current.m_nub = item.n.to_bool(parser.begin);
                        if (!item.l.is_null())
                            current.m_stepped = item.l.to_bool(parser.begin);
                        if (!item.d.is_null())
                            current.m_decal = item.d.to_bool(parser.begin);
                        if (!item.g.is_null())
                            current.m_ghost = item.g.to_bool(parser.begin);

                        current.m_y++;
                        current.m_x = current.m_rotation_x;
                    }

                    a++;
                }
            }
            else if (token.type == JSMN_OBJECT)
            {
                if (r > 1)
                {
                    // "keyboard metadata must the be first element"
                    return false;
                }
                r = parse(parser.begin, tokens, r, kb.m_meta);
			}
			else 
			{
				++r;
			}
        }
        return true;
    }

} // namespace kle