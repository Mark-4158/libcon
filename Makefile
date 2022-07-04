libcon.so : libcon.c
	cc -O2 -fPIC -lselinux -s -shared -o libcon.so libcon.c

install : libcon.so boxcon
	sudo install -s libcon.so /usr/local/lib64/
	sudo ldconfig
	sudo install boxcon /usr/local/bin/

install-all : install unconfined_mcs.pp
	sudo semodule -i unconfined_mcs.pp
	sudo semanage fcontext -a -f f -t dbusd_exec_t /usr/libexec/xdg-desktop-portal
	sudo restorecon -i /usr/libexec/xdg-desktop-portal
	sudo semanage boolean -m -1 secure_mode_policyload
	sudo mkdir -p /usr/share/selinux/packages/
	sudo cp unconfined_mcs.pp /usr/share/selinux/packages/

unconfined_mcs.mod : unconfined_mcs.te
	checkmodule -Mmo unconfined_mcs.mod unconfined_mcs.te

unconfined_mcs.pp : unconfined_mcs.mod
	semodule_package -o unconfined_mcs.pp -m unconfined_mcs.mod

remove :
	sudo rm -f /usr/local/bin/boxcon /usr/local/lib64/libcon.so
	sudo ldconfig
	semodule -r unconfined_mcs

clean :
	rm -f libcon.so unconfined_mcs.{pp,mod}
