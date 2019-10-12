
spec ?= contrib/fedora/*.spec
outdir ?= /var/tmp
contribdir = contrib/fedora

all: srpm

prereq:
	gzip --version >/dev/null || dnf -y install gzip
	tar --version >/dev/null || dnf -y install tar
	git --version || dnf -y install git-core
	rpmbuild --version || dnf -y install rpm-build

srpm: prereq
	sh ${contribdir}/mkarchive --copr
	sh ${contribdir}/mksnap -u ${spec}
	rpmbuild --define '_sourcedir .' --define "_srcrpmdir ${outdir}" --with copr -bs ${spec}
