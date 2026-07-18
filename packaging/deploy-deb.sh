#!/usr/bin/env bash
# Build will-server .deb, copy it to a remote host, and install with dpkg.
# Usage: ./packaging/deploy-deb.sh [user@host]
# Env:   DEPLOY_HOST (default remote if no arg), DEPLOY_REMOTE_DIR (default /tmp)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REMOTE="${1:-${DEPLOY_HOST:-}}"
REMOTE_DIR="${DEPLOY_REMOTE_DIR:-/tmp}"

if [[ -z "${REMOTE}" ]]; then
	echo "Usage: $0 user@host" >&2
	echo "Or set DEPLOY_HOST=user@host" >&2
	exit 1
fi

echo "==> Building package..."
"${ROOT}/packaging/build-deb.sh"

DEB="$(ls -1t "${ROOT}/build/deb-packaging"/will-server_*.deb | head -n1)"
[[ -n "${DEB}" && -f "${DEB}" ]] || { echo "No .deb found in build/deb-packaging" >&2; exit 1; }
DEB_BASENAME="$(basename "${DEB}")"
REMOTE_DEB="${REMOTE_DIR%/}/${DEB_BASENAME}"

echo "==> Copying ${DEB_BASENAME} to ${REMOTE}:${REMOTE_DEB}..."
scp "${DEB}" "${REMOTE}:${REMOTE_DEB}"

echo "==> Installing on ${REMOTE}..."
ssh -t "${REMOTE}" "sudo dpkg -i '${REMOTE_DEB}' && sudo systemctl --no-pager --full status will-server.service | head -n 20"

echo "==> Deployed ${DEB_BASENAME} to ${REMOTE}"
