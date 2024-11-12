```
MBR
Bios Parameter Block
File Allocation Table (FAT)
Root Directory Secion
Data Section
```

## MBR
- start at 0
- it contains information about partitions

## Bios Parameter Block
- start at 0x7e00
- bytes per sector
- sectors per cluster
- total sectors and other info
```
offset  size    field
0       3       jmp
3       8       OEM (=FRDOS4.1)
0xb     2       bytes per sector (=512)
0xd     1       sectors per cluster (=4), size of per cluster is 2KB
0xe     2       reserved sectors (=1)
0x10    1       number of fat table (=2)
0x11    2       root dir entries (=512)
0x13    2       total sectors (=0, if this is 0, we use extended sectors)
0x15    1       media descriptor (=0xF8, which is fixed media)
0x16    2       sectors per fat table (=200 = 0xc8)
0x18    2       sectors per track (=63)
0x1a    2       heads (=16)
0x1c    4       hidden sectors (=63)
0x20    4       extended sectors (=204561)
...

0x2b    11      label (=OS)
0x36    8       system type (=FAT16)

```


## File Allocation Table (FAT)
- number of fat table = 2
- 1st FAT start at 0x8000 = 0x7e00 + 1 (reserved sectors) * 512
- chains of links for a file (linked by cluster number)
- basic unit is cluster
- each cluster has its sectors
- The first two entries in the FAT Table are reserved
- FAT Table value
```
0x0000 = Cluster is free, not allocated to any file/directory
0x0002 – 0xFFEF  = used, next cluster in file
0xFFF7 = bad cluster
0xFFF8 – 0xFFFF = used, last cluster in file
```


## Root Directory Secion
- root directory location starts at 0x3a000
```
total sectors = 0x191 = 0xc8 * 2 (total sectors of FAT) + 1 (total sectors of bios parameter block)

total bytes of total sectors = 0x32200 = 512 * 0x191

0x3a000 = 0x7e00 + 0x32200

```

- files and directories
- size of each entry is 32 bytes
- first entry is volume label (=OS)
- directory entry
```
offset  size    field
0       8       file name
8       3       extension name
0xb     1       attribute
0xc     10      reserved
0x16    2       time
0x18    2       date
0x1a    2       starting cluster for data section
0x1c    4       file size (bytes)


first character of file name is allocation status:
unallocated 0x00
deleted     0xe5

attribute:
Read Only       0x01
Hidden file     0x02
System file     0x04
Volume label    0x08
Long file name  0x0f
Directory       0x10
Archieve        0x20

```

## Data Section
- start from cluster 2
- data section starts at 0x3e000
```
the size of root directory secion = 512 (total entries) * 32 (each entry is 32 bytes) = 0x4000

0x3e000 = 0x3a000 + 0x4000

```

- access file data
```
cluster of file
2,3,4,5,6,7,8,9,0xa,0xb

start address of file =  start of data section + (starting cluster - 2) * cluster size

since start from cluster 2, so (starting cluster - 2) is number of cluster 

```


