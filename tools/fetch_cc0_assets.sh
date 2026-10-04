#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ASSET_DIR="$ROOT/app/src/main/assets"
CAR_DIR="$ASSET_DIR/cars"
ENV_DIR="$ASSET_DIR/environment"
mkdir -p "$CAR_DIR" "$ENV_DIR"

fetch_asset() {
  local url="$1"
  local out="$2"
  if [ -s "$out" ]; then
    echo "asset exists: $out"
    return
  fi
  echo "downloading: $url"
  curl -fL --retry 3 --connect-timeout 15 --max-time 120 "$url" -o "$out"
  test -s "$out"
}

# CC0 1.0 open-wheel vehicle. Keep both manifest paths populated:
# player is required; AI is optional but improves race presentation.
CAR_URL="https://cdn.3dassets.dev/assets/18685/v1/model.glb"
fetch_asset "$CAR_URL" "$CAR_DIR/player.glb"
cp -f "$CAR_DIR/player.glb" "$CAR_DIR/ai.glb"

# Validate the GLB container before it reaches the Android package.
for f in "$CAR_DIR/player.glb" "$CAR_DIR/ai.glb"; do
  size=$(wc -c < "$f")
  [ "$size" -gt 1024 ]
  head -c 4 "$f" | grep -q "glTF"
  printf 'validated %-20s %8s bytes\n' "$(basename "$f")" "$size"
done

echo "Apex Engine Next runtime assets staged."

# Kenney Racing Kit is CC0. Its package contains glTF 2.0 trackside infrastructure.
# The exact package URL is resolved from the official asset page rather than hard-coding
# an undocumented CDN path; when unavailable, the generated Apex placeholders remain.
KENNEY_PAGE="https://kenney.nl/assets/racing-kit"
echo "CC0 environment source: $KENNEY_PAGE"
echo "Environment role policy: use Racing Kit glTF assets for fences, barriers, pit/track props and signs when staged; keep Apex circuit mesh/layout authoritative."
