# barukanOS

### Features

- Support for long mode
- Interrupt and exception handling
- Printing characters on the screen
- Memory manager with paging mechanism
- 2MB page size support under 4-level paging
- Timer handler for process management
- Process manager for process scheduling
- System call interface
- PS/2 keyboard driver
- Command-line interface for interacting with the OS kernel
- FAT16 file system support


### Build

- Step1: Installing Linux Toolchains on Windows

```sh
docker run --rm dockcross/linux-x64 > ./cross-linux-x64
chmod +x cross-linux-x64
sudo mv cross-linux-x64 /usr/bin
```

- Step2: Creating an OS Image (os.img)

```
1.Use bximage.exe (in the Bochs directory) to create an empty img (os.img) of 100MB
2.Use FreeDOS within Bochs to Format os.img to the FAT 16 file system
```

- Step3: Compiling Using Makefiles
```sh
make -f Makefile.windows lib
make -f Makefile.windows os-boot
make -f Makefile.windows build-kernel
make -f Makefile.windows os-loader
make -f Makefile.windows fat16-examples
```
> Use Makefile for Linux users \
> Use Makefile.windows for Windows users

- Step4: Placing Artifacts into the Mounted Drive

  - Artifacts to be placed:
  ```
  boot/kernel.bin
  examples/fs/user1.bin
  examples/fs/user2.bin
  examples/fs/ls
  examples/fs/data.bin
  ```
  - Use OSFMount to mount os.img and create a drive

- Step5: Starting the OS
```sh
make -f Makefile.windows qemu-os
```

[dockcross/linux-x64](https://github.com/dockcross/dockcross) \
[Bochs](https://bochs.sourceforge.io/) \
[FreeDOS](https://www.freedos.org/) \
[FAT 16](https://wiki.osdev.org/FAT#FAT_16) \
[OSFMount](https://www.osforensics.com/tools/mount-disk-images.html)



### Acknowledgements
[Write Your Own Operating System From Scratch - Step by Step](https://www.udemy.com/course/writing-your-own-operating-system-from-scratch/)
