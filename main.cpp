#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdint>


#define FTYP        "ftyp"
#define MOOV        "moov"
#define TRAK        "trak"
#define MDIA        "mdia"
#define MINF        "minf"
#define STBL        "stbl"
#define STCO        "stco"
#define MDAT        "mdat"

#define BUFF_SIZE   2000000
uint64_t fsize(const char* path);
uint8_t BoxInfo(int32_t* fd, uint64_t* fsize, uint64_t offset, const char* name, uint64_t* boxOffset, uint32_t* boxSize);

typedef struct mp4 mp4;

struct mp4
{
    uint32_t ftyp_size = 0;
    uint64_t ftyp_offset = 0;

    uint32_t moov_size = 0;
    uint64_t moov_offset = 0;

    uint32_t mdat_size = 0;
    uint64_t mdat_offset = 0;

    uint32_t stco_1_size = 0;
    uint64_t stco_1_offset = 0;
    uint32_t stco_1_chunk_num = 0;
    uint32_t* stco_1_chunks_offset;

    uint32_t stco_2_size = 0;
    uint64_t stco_2_offset = 0;
    uint32_t stco_2_chunk_num = 0;
    uint32_t* stco_2_chunks_offset;

};
int main()
{
    mp4 carrier_struct;

    int32_t  fd_carrier;
    uint64_t size_carrier;
    const char* path_carrier = "/valrond/1.mp4";

    int32_t  fd_hidden;
    const char* path_hidden = "/valrond/_1.mp4";


    int32_t  fd_vera;
    uint64_t size_vera;
    const char* path_vera = "/valrond/1.vc";

    uint8_t buff[BUFF_SIZE];
    uint32_t len;


    size_carrier = fsize(path_carrier);
    if (size_carrier < 1) return -1;

    fd_carrier = open(path_carrier, O_RDONLY);
    if (fd_carrier < 0) return -1;


    size_vera = fsize(path_vera);
    if (size_vera < 1) return -1;

    fd_vera = open(path_vera, O_RDONLY);
    if (fd_vera < 0) return -1;



    uint64_t offset = 0;

    BoxInfo(&fd_carrier, &size_carrier, offset, FTYP, &carrier_struct.ftyp_offset, &carrier_struct.ftyp_size);
    BoxInfo(&fd_carrier, &size_carrier, offset, MOOV, &carrier_struct.moov_offset, &carrier_struct.moov_size);
    BoxInfo(&fd_carrier, &size_carrier, offset, MDAT, &carrier_struct.mdat_offset, &carrier_struct.mdat_size);

    offset = carrier_struct.moov_offset;

    BoxInfo(&fd_carrier, &size_carrier, offset, STCO, &carrier_struct.stco_1_offset, &carrier_struct.stco_1_size);
    carrier_struct.stco_1_chunk_num = (carrier_struct.stco_1_size - 8) / 4;
    carrier_struct.stco_1_chunks_offset = new uint32_t[carrier_struct.stco_1_chunk_num];
    lseek(fd_carrier, carrier_struct.stco_1_offset + 8, SEEK_SET);
    for (uint32_t i = 0; i < carrier_struct.stco_1_chunk_num; i++)
    {
        read(fd_carrier, buff, 4);
        carrier_struct.stco_1_chunks_offset[i] = __builtin_bswap32(*((int32_t*)(buff)));
//        printf("\n%04x\n", carrier_struct.stco_1_chunks_offset[i]);
    }

    offset = carrier_struct.stco_1_offset + carrier_struct.stco_1_size;

    BoxInfo(&fd_carrier, &size_carrier, offset, STCO, &carrier_struct.stco_2_offset, &carrier_struct.stco_2_size);
    carrier_struct.stco_2_chunk_num = (carrier_struct.stco_2_size - 8) / 4;
    carrier_struct.stco_2_chunks_offset = new uint32_t[carrier_struct.stco_2_chunk_num];
    lseek(fd_carrier, carrier_struct.stco_2_offset + 8, SEEK_SET);
    for (uint32_t i = 0; i < carrier_struct.stco_2_chunk_num; i++)
    {
        read(fd_carrier, buff, 4);
        carrier_struct.stco_2_chunks_offset[i] = __builtin_bswap32(*((int32_t*)(buff)));
        //printf("\n%04x\n", carrier_struct.stco_2_chunks_offset[i]);
    }





    fd_hidden = open(path_hidden, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd_hidden < 0) return -1;


    uint32_t bytes_to_read;
    uint32_t new_stco_offset;
    uint32_t write_all = 0;

    /*ftyp*/
    bytes_to_read = carrier_struct.ftyp_size;
    lseek(fd_carrier, 0, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_carrier, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write (fd_hidden, buff, len);
    }


    bytes_to_read = carrier_struct.mdat_size;
    lseek(fd_carrier, carrier_struct.mdat_offset, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_carrier, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write (fd_hidden, buff, len);
    }

    /*moov header*/
    bytes_to_read = carrier_struct.stco_1_offset - carrier_struct.moov_offset + 8;
    lseek(fd_carrier, carrier_struct.moov_offset, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_carrier, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    /*stco 1*/
    for (uint32_t i = 0; i < carrier_struct.stco_1_chunk_num; i++)
    {
        if (i > 1) new_stco_offset = __builtin_bswap32(carrier_struct.stco_1_chunks_offset[i] - carrier_struct.stco_1_chunks_offset[2] + carrier_struct.ftyp_size + 8);
        else new_stco_offset = __builtin_bswap32(carrier_struct.stco_1_chunks_offset[i]);
        write_all += write (fd_hidden, &new_stco_offset, 4);
    }

    /*moov middle*/
    bytes_to_read = carrier_struct.stco_2_offset - carrier_struct.stco_1_offset - carrier_struct.stco_1_size + 8;
    lseek(fd_carrier, carrier_struct.stco_1_offset + carrier_struct.stco_1_size, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_carrier, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    /*stco 2*/
    for (uint32_t i = 0; i < carrier_struct.stco_2_chunk_num; i++)
    {
        if (i > 1) new_stco_offset = __builtin_bswap32(carrier_struct.stco_2_chunks_offset[i] - carrier_struct.stco_1_chunks_offset[2] + carrier_struct.ftyp_size + 8);
        else new_stco_offset = __builtin_bswap32(carrier_struct.stco_2_chunks_offset[i]);
        write_all += write (fd_hidden, &new_stco_offset, 4);
    }

    /*moov footer*/
    bytes_to_read = carrier_struct.moov_size - write_all;
    lseek(fd_carrier, carrier_struct.stco_2_offset + carrier_struct.stco_2_size, SEEK_SET);
    while (bytes_to_read > 0)
    {
        len = read(fd_carrier, buff, (bytes_to_read <= BUFF_SIZE) ? bytes_to_read : BUFF_SIZE);
        bytes_to_read -= len;
        write_all += write (fd_hidden, buff, len);
    }

    return 0;
}
uint8_t BoxInfo(int32_t* fd, uint64_t* fsize, uint64_t offset, const char* name, uint64_t* boxOffset, uint32_t* boxSize)
{
    uint8_t buff[4];

    lseek(*fd, offset, SEEK_SET);

    while(*fsize - offset > 0)
    {
        read(*fd, buff, 1);
        offset++;
        if (buff[0] == name[0])
        {
            read(*fd, buff, 1);
            offset++;
            if (buff[0] == name[1])
            {
                read(*fd, buff, 1);
                offset++;
                if (buff[0] == name[2])
                {
                    read(*fd, buff, 1);
                    offset++;
                    if (buff[0] == name[3])
                    {
                        *boxOffset = offset - 8;

                        lseek(*fd, *boxOffset, SEEK_SET);
                        read(*fd, buff, 4);
                        *boxSize = __builtin_bswap32(*((int32_t*)(buff)));
//                        printf("%llu\n", *boxOffset);
//                        printf("%u\n", *boxSize);
                        return 0;
                    }
                }
            }
        }
    }
    return -1;
};
uint64_t fsize (const char* path)
{
    struct stat finfo;
    if (!stat(path, &finfo)) return (uint64_t)finfo.st_size;
    return -1;
};


