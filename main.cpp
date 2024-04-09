/*
MIT License

Copyright (c) 2024 Aleksandr Yuschenko

Support
https://t.me/Alex_Yuschenko
https://vsyst.ru

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdint>
#include "vMP4.h"
#include "help.h"

#define BUFF_SIZE           5000000
#define OPN_VOL_HDR_SZ      0x10000
#define MAX_FILE_SIZE       4294000000
#define ERROR               "\nERROR\n%s\n"

int main(int argc, char** argv)
{
    const char* path_video;
    const char* path_vera;

    if ((argc == 2) and (!strcmp(argv[1],"-h")))
    {
        puts(HELP);
        return 0;
    }
    else if (argc == 3)
    {
        path_video = argv[1];
        path_vera = argv[2];
    }
    else
    {
        puts(PROMPT);
        return 0;
    }


    mp4_tree* tree_mp4;

    int32_t  fd_video, fd_vera, fd_hidden;
    uint64_t size_video, size_vera;

    uint32_t size_new_mdat;
    char* path_hidden;

    struct stat finfo;
    uint8_t buff[BUFF_SIZE];
    uint32_t len;

    printf("\nParsing video file...");

    tree_mp4 = mp4_tree_new();
    if (mp4_parse(path_video, tree_mp4))
    {
        printf(ERROR, "MP4 file parsing faild");
        return -1;
    }

    size_video =  (!stat(path_video, &finfo)) ? (uint64_t)finfo.st_size : 0;
    if (size_video < 1)
    {
        printf(ERROR, "MP4 file size is too low");
        return -1;
    }

    fd_video = open(path_video, O_RDONLY);
    if (fd_video < 0)
    {
        printf(ERROR, "MP4 file open");
        return -1;
    }
    printf("OK!\n");

    printf("Parsing veracrypt file...");
    size_vera = (!stat(path_vera, &finfo)) ? (uint64_t)finfo.st_size : 0;
    if (size_vera < OPN_VOL_HDR_SZ * 2)
    {
        printf(ERROR, "Veracrypt file size is too low");
        return -1;
    }

    if ((size_vera + size_video) > MAX_FILE_SIZE)
    {
        printf(ERROR, "Resulting file size cannot exceed 4294 MB");
        return -1;
    }

    fd_vera = open(path_vera, O_RDONLY);
    if (fd_vera < 0)
    {
        printf(ERROR, "Veracrypt file open");
        return -1;
    }
    printf("OK!\n");

    printf("Please wait.\nMaking resulting file...");

    len = strlen(path_vera) + 5;
    path_hidden = new char[len];
    sprintf(path_hidden, "%s.mp4", path_vera);

    fd_hidden = open(path_hidden, O_WRONLY | O_CREAT | O_TRUNC);
    delete []path_hidden;
    if (fd_hidden < 0)
    {
        printf(ERROR, "Resulting file create / open");
        return -1;
    }



    uint32_t bytes_to_read;
    uint32_t new_stco_offset;
    uint32_t write_all = 0;

    bytes_to_read = tree_mp4->ftyp.size;
    lseek(fd_video, tree_mp4->ftyp.offset, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_video, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write (fd_hidden, buff, len);
    }

    size_new_mdat = __builtin_bswap32((uint32_t)size_vera + tree_mp4->mdat.size - tree_mp4->ftyp.size - 8);
    write (fd_hidden, &size_new_mdat, 4);

    write (fd_hidden, MDAT, 4);


    len = OPN_VOL_HDR_SZ - 8 - tree_mp4->ftyp.size;
    for (uint32_t i = 0; i < len; i++) buff[i] = rand() % 255;
    write (fd_hidden, buff, len);

    bytes_to_read = size_vera - OPN_VOL_HDR_SZ;
    lseek(fd_vera, OPN_VOL_HDR_SZ, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_vera, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write (fd_hidden, buff, len);
    }

    bytes_to_read = tree_mp4->mdat.size - 8;
    lseek(fd_video, tree_mp4->mdat.offset + 8, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_video, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write (fd_hidden, buff, len);
    }

    bytes_to_read = tree_mp4->moov.traks[0].mdia.minf.stbl.stco.offset - tree_mp4->moov.offset + 8;
    lseek(fd_video, tree_mp4->moov.offset, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_video, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    for (uint32_t i = 0; i < tree_mp4->moov.traks[0].mdia.minf.stbl.stco.chunks_num; i++)
    {
        if (i > 1) new_stco_offset = __builtin_bswap32(tree_mp4->moov.traks[0].mdia.minf.stbl.stco.chunks_offsets[i] - tree_mp4->moov.traks[0].mdia.minf.stbl.stco.chunks_offsets[2] + (uint32_t)size_vera);
        else new_stco_offset = __builtin_bswap32(tree_mp4->moov.traks[0].mdia.minf.stbl.stco.chunks_offsets[i]);
        write_all += write (fd_hidden, &new_stco_offset, 4);
    }

    bytes_to_read = tree_mp4->moov.traks[1].mdia.minf.stbl.stco.offset - tree_mp4->moov.traks[0].mdia.minf.stbl.stco.offset - tree_mp4->moov.traks[0].mdia.minf.stbl.stco.size + 8;
    lseek(fd_video, tree_mp4->moov.traks[0].mdia.minf.stbl.stco.offset + tree_mp4->moov.traks[0].mdia.minf.stbl.stco.size, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_video, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    for (uint32_t i = 0; i < tree_mp4->moov.traks[1].mdia.minf.stbl.stco.chunks_num; i++)
    {
        if (i > 1) new_stco_offset = __builtin_bswap32(tree_mp4->moov.traks[1].mdia.minf.stbl.stco.chunks_offsets[i] - tree_mp4->moov.traks[0].mdia.minf.stbl.stco.chunks_offsets[2] + (uint32_t)size_vera);
        else new_stco_offset = __builtin_bswap32(tree_mp4->moov.traks[1].mdia.minf.stbl.stco.chunks_offsets[i]);
        write_all += write (fd_hidden, &new_stco_offset, 4);
    }

    bytes_to_read = tree_mp4->moov.size - write_all;
    lseek(fd_video, tree_mp4->moov.traks[1].mdia.minf.stbl.stco.offset + tree_mp4->moov.traks[1].mdia.minf.stbl.stco.size, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_video, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    close (fd_video);
    close (fd_vera);
    close (fd_hidden);

    unlink(path_vera);
    unlink(path_video);

    mp4_tree_free(tree_mp4);

    printf("OK!\n");
    return 0;
}


