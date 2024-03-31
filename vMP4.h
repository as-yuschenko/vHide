#ifndef VMP4_H_INCLUDED
#define VMP4_H_INCLUDED

#include <cstdint>

#define FTYP        "ftyp"
#define MOOV        "moov"
#define TRAK        "trak"
#define MDIA        "mdia"
#define MINF        "minf"
#define STBL        "stbl"
#define STCO        "stco"
#define MDAT        "mdat"
#define FREE        "free"

typedef struct b_ftyp b_ftyp;
struct b_ftyp
{
    uint32_t size = 0;
    uint64_t offset = 0;
};

typedef struct b_mdat b_mdat;
struct b_mdat
{
    uint32_t size = 0;
    uint64_t offset = 0;
};

typedef struct b_free b_free;
struct b_free
{
    uint32_t size = 0;
    uint64_t offset = 0;
};

typedef struct b_stco b_stco;
struct b_stco
{
    uint32_t size = 0;
    uint64_t offset = 0;
    uint32_t chunks_num = 0;
    uint32_t* chunks_offsets = nullptr;
};

typedef struct b_stbl b_stbl;
struct b_stbl
{
    uint32_t size = 0;
    uint64_t offset = 0;
    b_stco stco;
};

typedef struct b_minf b_minf;
struct b_minf
{
    uint32_t size = 0;
    uint64_t offset = 0;
    b_stbl stbl;
};

typedef struct b_mdia b_mdia;
struct b_mdia
{
    uint32_t size = 0;
    uint64_t offset = 0;
    b_minf minf;
};

typedef struct b_trak b_trak;
struct b_trak
{
    uint32_t size = 0;
    uint64_t offset = 0;
    b_mdia mdia;
};

typedef struct b_moov b_moov;
struct b_moov
{
    uint32_t size = 0;
    uint64_t offset = 0;
    b_trak* traks = nullptr;
    uint8_t traks_num = 0;

};

typedef struct  mp4_tree mp4_tree;
struct mp4_tree
{
    b_ftyp ftyp;
    b_moov moov;
    b_mdat mdat;
    b_free free;
};

mp4_tree*   mp4_tree_new();
uint8_t     mp4_parse(const char* path, mp4_tree* p_mp4_tree);
void        mp4_tree_show(mp4_tree* p_mp4_tree);
void        mp4_tree_free(mp4_tree* p_mp4_tree);

#endif // VMP4_H_INCLUDED
