.PHONY: package clean

package: PKGBUILD
	makepkg -i

clean:
	rm -rf cantata-git*.tar* cantata-git/ pkg/ src/
