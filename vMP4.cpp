#include "vMP4.h"

#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


mp4_tree*   mp4_tree_new()
{
    return new mp4_tree;
};
void        mp4_tree_free(mp4_tree* p_mp4_tree)
{
    for (uint8_t i = 0; i < p_mp4_tree->moov.traks_num; i++)
        if (p_mp4_tree->moov.traks[i].mdia.minf.stbl.stco.chunks_offsets)
            delete []p_mp4_tree->moov.traks[i].mdia.minf.stbl.stco.chunks_offsets;

    if (p_mp4_tree->moov.traks) delete []p_mp4_tree->moov.traks;
    delete p_mp4_tree;
    return;
};
uint8_t     mp4_parse(const char* path, mp4_tree* p_mp4_tree)
{
    uint32_t fd;
    uint64_t flen = 0;
    uint8_t buff[4];
    uint64_t offset;
    uint32_t box_size;
    struct stat finfo;


    if (!stat(path, &finfo))
        flen = (uint64_t)finfo.st_size;

    if (flen < 1) return -1;

    fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    offset = 0;
    while (offset < flen)
    {
        lseek(fd, offset, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;
        box_size = __builtin_bswap32(*((int32_t*)(buff)));

        lseek(fd, offset + 4, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;

        if      (!memcmp(FTYP, buff, 4))
        {
            p_mp4_tree->ftyp.offset = offset;
            p_mp4_tree->ftyp.size = box_size;

        }
        else if (!memcmp(MOOV, buff, 4))
        {
            p_mp4_tree->moov.offset = offset;
            p_mp4_tree->moov.size = box_size;
        }
        else if (!memcmp(MDAT, buff, 4))
        {
            p_mp4_tree->mdat.offset = offset;
            p_mp4_tree->mdat.size = box_size;

        }
        else if (!memcmp(FREE, buff, 4))
        {
            p_mp4_tree->mdat.offset = offset;
            p_mp4_tree->mdat.size = box_size;
        }

        offset += box_size;
    }

    offset = p_mp4_tree->moov.offset + 8;
    while (offset < p_mp4_tree->moov.offset + p_mp4_tree->moov.size)
    {
        lseek(fd, offset, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;
        box_size = __builtin_bswap32(*((int32_t*)(buff)));

        lseek(fd, offset + 4, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;

        if (!memcmp(TRAK, buff, 4)) p_mp4_tree->moov.traks_num++;
        offset += box_size;
    }

    p_mp4_tree->moov.traks = new b_trak[p_mp4_tree->moov.traks_num];

    uint8_t trak_num = 0;

    offset = p_mp4_tree->moov.offset + 8;
    while (offset < p_mp4_tree->moov.offset + p_mp4_tree->moov.size)
    {
        lseek(fd, offset, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;
        box_size = __builtin_bswap32(*((int32_t*)(buff)));

        lseek(fd, offset + 4, SEEK_SET);
        if (read(fd, buff, 4) != 4) return -1;

        if (!memcmp(TRAK, buff, 4))
        {
            p_mp4_tree->moov.traks[trak_num].offset = offset;
            p_mp4_tree->moov.traks[trak_num].size = box_size;
            trak_num++;
        }

        offset += box_size;
    }


    for (uint8_t t = 0; t < p_mp4_tree->moov.traks_num; t++)
    {
        offset = p_mp4_tree->moov.traks[t].offset + 8;
        while (offset < p_mp4_tree->moov.traks[t].offset + p_mp4_tree->moov.traks[t].size)
        {
            lseek(fd, offset, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;
            box_size = __builtin_bswap32(*((int32_t*)(buff)));

            lseek(fd, offset + 4, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;

            if (!memcmp(MDIA, buff, 4))
            {
                p_mp4_tree->moov.traks[t].mdia.offset = offset;
                p_mp4_tree->moov.traks[t].mdia.size = box_size;
            }

            offset += box_size;
        }

        offset = p_mp4_tree->moov.traks[t].mdia.offset + 8;
        while (offset < p_mp4_tree->moov.traks[t].mdia.offset + p_mp4_tree->moov.traks[t].mdia.size)
        {
            lseek(fd, offset, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;
            box_size = __builtin_bswap32(*((int32_t*)(buff)));

            lseek(fd, offset + 4, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;

            if (!memcmp(MINF, buff, 4))
            {
                p_mp4_tree->moov.traks[t].mdia.minf.offset = offset;
                p_mp4_tree->moov.traks[t].mdia.minf.size = box_size;
            }

            offset += box_size;
        }

        offset = p_mp4_tree->moov.traks[t].mdia.minf.offset + 8;
        while (offset < p_mp4_tree->moov.traks[t].mdia.minf.offset + p_mp4_tree->moov.traks[t].mdia.minf.size)
        {

            lseek(fd, offset, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;
            box_size = __builtin_bswap32(*((int32_t*)(buff)));

            lseek(fd, offset + 4, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;

            if (!memcmp(STBL, buff, 4))
            {
                p_mp4_tree->moov.traks[t].mdia.minf.stbl.offset = offset;
                p_mp4_tree->moov.traks[t].mdia.minf.stbl.size = box_size;
            }

            offset += box_size;
        }

        offset = p_mp4_tree->moov.traks[t].mdia.minf.stbl.offset + 8;
        while (offset < p_mp4_tree->moov.traks[t].mdia.minf.stbl.offset + p_mp4_tree->moov.traks[t].mdia.minf.stbl.size)
        {
            lseek(fd, offset, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;
            box_size = __builtin_bswap32(*((int32_t*)(buff)));

            lseek(fd, offset + 4, SEEK_SET);
            if (read(fd, buff, 4) != 4) return -1;

            if (!memcmp(STCO, buff, 4))
            {
                p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.offset = offset;
                p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.size = box_size;
            }

            offset += box_size;
        }

        p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_num = (p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.size - 8) / 4;
        p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_offsets = new uint32_t [p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_num];
        lseek(fd, p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.offset + 8, SEEK_SET);
        for (uint32_t i = 0; i < p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_num; i++)
        {
            read(fd, buff, 4);
            p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_offsets[i] = __builtin_bswap32(*((int32_t*)(buff)));
        }

    }

    close(fd);
    return 0;
};
void        mp4_tree_show(mp4_tree* p_mp4_tree)
{
    printf("ftyp offset:%llu size:%u\n", p_mp4_tree->ftyp.offset, p_mp4_tree->ftyp.size);
    printf("mdat offset:%llu size:%u\n", p_mp4_tree->mdat.offset, p_mp4_tree->mdat.size);
    printf("free offset:%llu size:%u\n", p_mp4_tree->free.offset, p_mp4_tree->free.size);
    printf("moov offset:%llu size:%u\n", p_mp4_tree->moov.offset, p_mp4_tree->moov.size);
    for (uint8_t t = 0; t < p_mp4_tree->moov.traks_num; t++)
    {
        printf("\ttrak%u offset:%llu size:%u\n\n", t, p_mp4_tree->moov.traks[t].offset, p_mp4_tree->moov.traks[t].size);

        for (uint32_t c = 0; c < p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_num; c++)
            printf("\t\t\tchunk %u : %x\n", c, p_mp4_tree->moov.traks[t].mdia.minf.stbl.stco.chunks_offsets[c]);

    }
};
