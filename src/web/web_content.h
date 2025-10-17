#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

#include "html_styles.h"
#include "html_scripts.h"
#include "html_index.h"

String buildHTML() {
  String html;
  html.reserve(32000);
  html = HTML_INDEX;
  String css = HTML_STYLES;
  String js = HTML_SCRIPTS;
  html.replace("%CSS%", css);
  html.replace("%JAVASCRIPT%", js);
  return html;
}

#endif
