- Linux Tool chain installation on Windows

```sh
docker run --rm dockcross/linux-x64 > ./cross-linux-x64
chmod +x cross-linux-x64
sudo mv cross-linux-x64 /usr/bin
```

- os.img: 
```
step1: create an empty img (os.img) of 100MB
use bximage (in bochs directory)

step2: format to FAT16 with FreeDOS in bochs
```

- os-boot (Makefile): write boot.bin into os.img

- os-loader (Makefile): write loader.bin into os.img

- os.img (as a drive)
```
boot/kernel.bin  // kernel
examples/fs/user1.bin // user program 1
examples/fs/user2.bin // user program 2
examples/fs/ls        // user program 3
examples/fs/data.bin  // input data for user program 2
```

- the order of compilation
```
1.lib
2.os-boot
3.build-kernel
4.os-loader
5.fat16-examples
```

- max size
```
1.boot.bin: (1 * 512) byes
1.loader.bin: (30 * 512) bytes 
2.each user program: (10 * 512) bytes
```