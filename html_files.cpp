#include "ui_screens.h"

const char *settings_html()
{
    const char *html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width">
<title>T-Watch Config</title>
<body>T-Watch Config</body>
</html>
    )rawliteral";

    return html;
}

void save_html_files_to_disk()
{
    if (!file_exists(SETTINGS_HTML)) {
        const char *settings_html_content = settings_html();
        write_file(SETTINGS_HTML, settings_html_content); 
    }
}