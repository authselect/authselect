#
# Make source RPM.
#

set -x

builddir=`mktemp -d -p .`
outdir=${1:-$srcdir}
trap "rm -fr $builddir" EXIT

autoreconf -if
pushd "$builddir"
../configure --enable-silent-rules
make srpm
mv rpmbuild/SRPMS/* "$outdir"
popd

exit 0
