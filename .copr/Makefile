
spec ?= contrib/fedora/*.spec
outdir ?= /var/tmp
contribdir = contrib/fedora

all: srpm

srpm:
	sh ${contribdir}/mkarchive
	sh ${contribdir}/mksnap -u ${spec}
	rpmbuild --version || dnf -y install rpm-build
	rpmbuild --define '_sourcedir .' --define "_srcrpmdir ${outdir}" --define 'pkgversion git' -bs ${spec}
