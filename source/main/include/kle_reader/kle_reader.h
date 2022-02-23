#ifndef __KLE_READER_H__
#define __KLE_READER_H__

#define KLE_LABEL_MAX 12
#define KLE_KEY_MAX 256

namespace kle
{
    struct color_t
    {
        inline color_t()
            : m_r(0)
            , m_g(0)
            , m_b(0)
            , m_a(255)
        {
        }
        inline color_t(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
            : m_r(r)
            , m_g(g)
            , m_b(b)
            , m_a(a)
        {
        }

        bool operator==(const color_t& rhs) const { return m_r == rhs.m_r && m_g == rhs.m_g && m_b == rhs.m_b && m_a == rhs.m_a; }
        bool operator!=(const color_t& rhs) const { return !(*this == rhs); }

        unsigned char m_r;
        unsigned char m_g;
        unsigned char m_b;
        unsigned char m_a;
    };

    struct str_t
    {
        inline str_t()
            : m_begin(-1)
            , m_end(-1)
        {
        }
        inline str_t(int begin, int end)
            : m_begin(begin)
            , m_end(end)
        {
        }

        bool is_null() const { return m_begin == -1 && m_begin == m_end; }

        color_t to_color(const char* json) const;
        int     to_int(const char* json) const;
        bool    to_bool(const char* json) const;

        int m_begin;
        int m_end;
    };

    struct bkgrnd_t
    {
        str_t m_name;
        str_t m_style;
    };

    struct kb_metadata_t
    {
        str_t    m_author;
        color_t  m_backcolor;
        bkgrnd_t m_background;
        str_t    m_name;
        str_t    m_notes;
        str_t    m_radii;
        str_t    m_switchBrand;
        str_t    m_switchMount;
        str_t    m_switchType;
    };

    struct key_t
    {
        color_t m_color;
        str_t   m_labels[KLE_LABEL_MAX];
        color_t m_textColor[KLE_LABEL_MAX];
        int     m_textSize[KLE_LABEL_MAX];
        color_t m_default_textColor;
        int     m_default_textSize;
        int     m_x;
        int     m_y;
        int     m_width;
        int     m_height;
        int     m_x2;
        int     m_y2;
        int     m_width2;
        int     m_height2;
        int     m_rotation_x;
        int     m_rotation_y;
        int     m_rotation_angle;
        bool    m_decal;
        bool    m_ghost;
        bool    m_stepped;
        bool    m_nub;
        str_t   m_profile;
    };

    struct kb_t
    {
        kb_metadata_t m_meta;
        int           m_nb_keys;
        key_t         m_keys[KLE_KEY_MAX];
    };

    bool read(const char* filename, kb_t& kb);

} // namespace kle

#endif // __KLE_READER_H__