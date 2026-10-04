#!/usr/bin/env bash
# Enforce the widget portability rule: widget code may not reach into the
# platform's internals. Allowed includes from main/widgets/**:
#   widgets/*, ui/page_id.h, ui/renderers/rawdraw/* (renderer base + own
#   renderer headers until Phase 2 moves them), rawdraw/*, common/*,
#   settings.h, ESP-IDF / system headers.
# Forbidden: application.h, boards/*, ui/rawdraw_ui_manager.h, display/*,
#   audio/*, protocols/*, streaming/*.
set -euo pipefail
cd "$(dirname "$0")/.."

violations=$(grep -rn --include='*.cc' --include='*.h' -E \
  '#include +"(application\.h|boards/|ui/rawdraw_ui_manager\.h|ui/ui_manager\.h|display/|audio/|protocols/|streaming/)' \
  main/widgets/ || true)

if [ -n "$violations" ]; then
  echo "Widget dependency violations (widgets must stay portable):"
  echo "$violations"
  exit 1
fi
echo "Widget dependency check passed."
