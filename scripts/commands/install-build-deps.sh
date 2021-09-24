#
# Install build dependencies.
#

set -x

if [ ! -f "/etc/os-release" ]; then
    die "Unknown release, please install dependencies manualy."
fi

. /etc/os-release

case "$ID" in
  fedora)
    [ ! -f "rpm/authselect.spec.in" ] && die "Missing file: rpm/authselect.spec.in"
    [ -f "authselect.spec" ] && die "./authselect.spec already exist, delete it to continue."

    sed -E "s/@\w+@/dummy/g" rpm/authselect.spec.in > authselect.spec
    dnf install -y "dnf-command(builddep)"
    dnf builddep -y --spec ./authselect.spec
    rm authselect.spec
    ;;
  *)
    die "Unknown release, please install dependencies manualy."
    ;;
esac
