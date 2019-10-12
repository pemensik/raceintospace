
outdir ?= /var/tmp
contribdir = contrib/fedora
spec ?= ${contribdir}/*.spec

all: srpm

prereq:
	gzip --version >/dev/null || dnf -y install gzip
	tar --version >/dev/null || dnf -y install tar
	git --version || dnf -y install git-core
	rpmbuild --version || dnf -y install rpm-build

# Make a copy of spec and enable copr inside
srpm: prereq
	sh ${contribdir}/mkarchive --copr
	cp ${spec} ${outdir}
	sed -i -e 's/^%bcond_with\s\+copr/%bcond_without copr/' ${outdir}/*.spec
	sh ${contribdir}/mksnap -u ${outdir}/*.spec
	rpmbuild --define '_sourcedir .' --define "_srcrpmdir ${outdir}" -bs ${outdir}/*.spec
