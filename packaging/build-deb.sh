#!/usr/bin/env bash
# Build a .deb with will-server and a systemd service (will-server.service).
# Requires: cmake, build-essential, dpkg-dev, and dpkg-deb (usually from dpkg).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION="${VERSION:-}"
if [[ -z "${VERSION}" ]]; then
	VERSION="$(
		sed -n 's/^[[:space:]]*static constexpr const char\* Version = "\([^"]*\)".*/\1/p' \
			"${ROOT}/src/server/willserver.h" | head -n1
	)"
fi
[[ -n "${VERSION}" ]] || { echo "Could not detect version from src/server/willserver.h" >&2; exit 1; }

if command -v dpkg-architecture >/dev/null 2>&1; then
	ARCH="$(dpkg-architecture -qDEB_HOST_ARCH 2>/dev/null || dpkg-architecture -qDEB_BUILD_ARCH)"
else
	case "$(uname -m)" in
		x86_64) ARCH=amd64 ;;
		aarch64|arm64) ARCH=arm64 ;;
		armv7l) ARCH=armhf ;;
		*) ARCH="$(uname -m)" ;;
	esac
fi

WORKDIR="${ROOT}/build/deb-packaging"
STAGE="${WORKDIR}/root"
DEBIAN="${STAGE}/DEBIAN"

rm -rf "${WORKDIR}"
mkdir -p "${DEBIAN}" "${STAGE}/usr/bin" "${STAGE}/lib/systemd/system" \
	"${STAGE}/usr/share/doc/will-server"

echo "==> Configuring and building will-server (Release)..."
cmake -S "${ROOT}" -B "${WORKDIR}/cmake-build" -DCMAKE_BUILD_TYPE=Release
cmake --build "${WORKDIR}/cmake-build" --target will-server -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

install -m755 "${WORKDIR}/cmake-build/will-server" "${STAGE}/usr/bin/will-server"
install -m644 "${ROOT}/packaging/deb/will-server.service" "${STAGE}/lib/systemd/system/will-server.service"
install -m644 "${ROOT}/packaging/deb/README.Debian" "${STAGE}/usr/share/doc/will-server/README.Debian"

DEB_NAME="will-server_${VERSION}_${ARCH}.deb"
DEB_PATH="${WORKDIR}/${DEB_NAME}"

cat >"${DEBIAN}/control" <<EOF
Package: will-server
Source: will
Version: ${VERSION}
Section: net
Priority: optional
Architecture: ${ARCH}
Maintainer: Unmaintained snapshot <root@localhost>
Depends: adduser, libc6, libgcc-s1 | libgcc1, libsqlite3-0, libstdc++6
Description: Will Messenger TCP server
 Standalone server binary for the Will messenger protocol.
 Installs systemd unit will-server.service (user will-server).
EOF

cat >"${DEBIAN}/postinst" <<'EOS'
#!/bin/sh
set -e
case "$1" in
	configure)
		if ! getent passwd will-server >/dev/null 2>&1; then
			adduser --quiet --system --group --no-create-home \
				--shell /usr/sbin/nologin --disabled-login will-server
		fi
		install -d -o will-server -g will-server -m 0750 /var/lib/will-server
		if [ -d /run/systemd/system ] && [ -x "$(command -v systemctl)" ]; then
			systemctl daemon-reload
			systemctl enable will-server.service >/dev/null 2>&1 || true
			systemctl restart will-server.service || systemctl start will-server.service || true
		fi
		;;
esac
exit 0
EOS

cat >"${DEBIAN}/prerm" <<'EOS'
#!/bin/sh
set -e
case "$1" in
	remove|upgrade|deconfigure)
		if [ -d /run/systemd/system ] && [ -x "$(command -v systemctl)" ]; then
			systemctl stop will-server.service >/dev/null 2>&1 || true
			systemctl disable will-server.service >/dev/null 2>&1 || true
		fi
		;;
esac
exit 0
EOS

cat >"${DEBIAN}/postrm" <<'EOS'
#!/bin/sh
set -e
case "$1" in
	abort-install|abort-upgrade) ;;
	remove|purge)
		if [ -d /run/systemd/system ] && [ -x "$(command -v systemctl)" ]; then
			systemctl daemon-reload >/dev/null 2>&1 || true
		fi
		;;
esac
exit 0
EOS

chmod 755 "${DEBIAN}/postinst" "${DEBIAN}/prerm" "${DEBIAN}/postrm"

echo "==> Building ${DEB_NAME}..."
dpkg-deb --root-owner-group --build "${STAGE}" "${DEB_PATH}"

echo "==> Done: ${DEB_PATH}"
ls -lh "${DEB_PATH}"
