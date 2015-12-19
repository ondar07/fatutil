mount:
	### bs is size of block (in bytes), count is number of blocks (в сумме 16 Mb)
	dd if=/dev/zero of=fatdisk bs=1048576 count=16
	mkfs.vfat -c -v -f2 -n FAT16 -r224 -F16 fatdisk
	### создаем точку монтирования 
	mkdir fatmnt
	sudo mount -o loop fatdisk fatmnt
	sync
	sync

# для повторного монтирования
repeat:
	sudo mount -o loop ../fatdisk ../fatmnt

umount:
	sudo umount ../fatmnt

clean:
	rm -fr ../fatmnt ../fatdisk 
